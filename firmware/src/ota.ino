
#include <ArduinoOTA.h>

void setup_ota_upgrades() {
  ArduinoOTA.begin();

  /* We have no display to show progress, hence I disable all this:

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });

  ArduinoOTA.onEnd([]() {
  });

  ArduinoOTA.onError([](ota_error_t error) {
  });
  */
}
