
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

#include "password.h"


#define BUTTON_MAKE 0

#define RELAY_HEATER 12

#define SHORT_PRESS 1
#define LONG_PRESS 1500


// 6 minutes when cold
#define COFFEE_MAKING_SECONDS  (6*60)
#define IGNORE_STOP 2

#define COFFEE_TIME_WEEKEND_H 8
#define COFFEE_TIME_H  6
#define COFFEE_TIME_M  15

// here it's GMT+2 now in summertime, ideally we should discover
// DST changes at least
#define TIMEZONE 1

void blink(){
    digitalWrite(RELAY_HEATER, HIGH);
    delay(500);
    digitalWrite(RELAY_HEATER, LOW);
    delay(500);
}

// This function servers for testing the stability of the electronics
// because load switching can generate noise, and reboot the current board
void foreverblink() {
    while (true) {
        digitalWrite(RELAY_HEATER, HIGH);
        delay(200);
        digitalWrite(RELAY_HEATER, LOW);
        delay(200);
    }
}
void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(1);
  WiFi.mode(WIFI_STA);
  WiFi.begin(AP,PASS);

  digitalWrite(RELAY_HEATER, LOW);
  pinMode(RELAY_HEATER, OUTPUT);
  pinMode(BUTTON_MAKE, INPUT_PULLUP);


  setup_ota_upgrades();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  for (int i=0;i<100;i++) {
    ArduinoOTA.handle();
    delay(25);
  }

  ntp_begin();
  blink();

}

int button_make() {
    int ms = 0;
    while(digitalRead(BUTTON_MAKE) == 0) {
        ms += 50;
        delay(50);
        if (ms>LONG_PRESS) break;
    }
    return ms;
}

void make_coffee(){
    int remaining_time;
    digitalWrite(RELAY_HEATER, HIGH);
    delay(IGNORE_STOP * 1000);

    remaining_time = COFFEE_MAKING_SECONDS - IGNORE_STOP;
    while(remaining_time > 0) {
        delay(1000);
        // handle ABORT request, if user pushes button, we assume
        // he wants to stop the coffee machine
        if (button_make()) break;
        remaining_time -= 1;
    }
    digitalWrite(RELAY_HEATER, LOW);
    delay(2000);

    // make sure the make button is not pushed before we leave
    // the function (to avoid re-entering quickly)
    while(button_make()) {
        delay(1000);
        ArduinoOTA.handle();
    }

}

int adjust_timezone(int h) {
    return (h + TIMEZONE) % 24;
}

bool is_coffee_time() {
    int wd = weekday();
    int h = adjust_timezone(hour());
    int m = minute(); // race conditions, I know .. ;)
    int coffee_h = COFFEE_TIME_H;

    if (wd == 1  || wd == 7 ) // on sunday and saturday
       coffee_h = COFFEE_TIME_WEEKEND_H;

    return ((h == coffee_h) &&
            (m == COFFEE_TIME_M));
}

bool  i_have_coffee = true; /* start at true, to recover from board electrica/WDT issues */
void loop() {

    int button_time;

    ArduinoOTA.handle();

    button_time = button_make();

    if (button_time > LONG_PRESS) {
        Serial.println("Making coffee\r\n");
        make_coffee();
        i_have_coffee = false;
    } else if (button_time > SHORT_PRESS) {
        Serial.println("Ready for coffee\r\n");
        i_have_coffee = true;
        blink();
    }

    if (is_coffee_time() && i_have_coffee) {
       Serial.println("It's coffee time\r\n");
       make_coffee();
       i_have_coffee = false;
    }

}
