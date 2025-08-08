#include <TinyGPSPlus.h>
#define _Gps_RX 16
#define _Gps_TX 17
#define _Gps_serial_Baud 9600
HardwareSerial _Gps_serial(1);
String _Gps_Payload = "";
uint32_t timer = 0;

TinyGPSPlus gps;

void displayInfo() {
  String dateString = "";
  String timeString = "";
  float lon = 0;
  float lat = 0;
  int alt = 0;
  int sat = 0;
  if (gps.date.isValid()) {
    dateString += String(gps.date.day(), DEC);
    dateString += "/";
    dateString += String(gps.date.month(), DEC);
    dateString += "/";
    dateString += String(gps.date.year(), DEC);
  }
  if (gps.time.isValid()) {
    if (gps.time.hour() < 10) { timeString += "0"; }
    timeString += String(gps.time.hour(), DEC);
    timeString += ":";
    if (gps.time.minute() < 10) { timeString += "0"; }
    timeString += String(gps.time.minute(), DEC);
    timeString += ":";
    if (gps.time.second() < 10) { timeString += "0"; }
    timeString += String(gps.time.second(), DEC);
  }
  if (gps.location.isValid()) {
    lat = gps.location.lat();
    lon = gps.location.lng();
  }
  if (gps.satellites.isValid()) {
    sat = gps.satellites.value();
  }
  if (gps.altitude.isValid()) {
    alt = gps.altitude.meters();
  }
  DynamicJsonDocument doc(512);
  doc["hbt"] = (millis() / 1000);
  doc["lat"] = round(lat * 10000.0) / 10000.0;
  doc["lon"] = round(lon * 10000.0) / 10000.0;
  doc["alt"] = alt;
  doc["sat"] = sat;
  doc["Date"] = dateString;
  doc["Time"] = timeString;
  serializeJson(doc, _Gps_Payload);
  Serial.println(_Gps_Payload);
}

void _Gps_tg_setup() {
  _Gps_serial.begin(_Gps_serial_Baud, SERIAL_8N1, _Gps_RX, _Gps_TX);
  delay(1000);
  timer = millis();
}

void _Gps_tg_loop() {
  while (_Gps_serial.available() > 0) {
    if (gps.encode(_Gps_serial.read())) {
      displayInfo();
    }
  }
}