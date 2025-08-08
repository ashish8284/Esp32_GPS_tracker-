#include <ArduinoJson.h>
#include "esp_heap_caps.h"
// #include "_Gps.h"
#include "_Gps_tg.h"
#include "_Gprs.h"

#define LED_PIN 13
int ledStatus = LOW;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  // _Gps_setup();
  _Gps_tg_setup();
  _Gprs_setup();
}

void loop() {
  Serial.print("Gps_Loop Start--> ");
  // _Gps_loop();
  _Gps_tg_loop();
  Serial.println("Gps_Loop End");
  Serial.print("Gprs_Loop Start--> ");
  _Gprs_loop();
  Serial.println("Gprs_Loop End");
  Serial.print("Diag_print Start--> ");
  one_sec_call();
   Serial.println("Diag_print End");
}

void one_sec_call() {
  if (millis() - onesec_elspd > 2000) {
    onesec_elspd = millis();
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, _Gps_Payload);
    if (error) {
      return;
    } else {
      doc["hbt"] = (millis() / 1000);
      JsonArray sts = doc.createNestedArray("sts");
      sts.add(_Gprs_network_sts);
      sts.add(_Gprs_gprs_sts);
      sts.add(_Gprs_mqtt_sts);
      sts.add(_Gprs_reconnect_count);
      sts.add(ESP.getFreeHeap());
      sts.add(ESP.getMinFreeHeap());
      sts.add(heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
      sts.add(heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
      String output;
      serializeJson(doc, output);
      Serial.println(output);
    }
  }
}