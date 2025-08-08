#include <Adafruit_GPS.h>
#define _Gps_RX 16
#define _Gps_TX 17
#define _Gps_serial_Baud 9600
bool _Gps_parse_flt = 0;
HardwareSerial _Gps_serial(1);
Adafruit_GPS GPS(&_Gps_serial);
#define GPSECHO false
String _Gps_Payload = "";
uint32_t timer = 0;
float convertToDecimalDegrees(float dmm) {
  int degrees = int(dmm / 100);
  float minutes = dmm - (degrees * 100);
  return degrees + (minutes / 60.0);
}

void _Gps_setup() {
  _Gps_serial.begin(_Gps_serial_Baud, SERIAL_8N1, _Gps_RX, _Gps_TX);
  delay(5000);
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
  _Gps_serial.println(PMTK_Q_RELEASE);
  timer = millis();
}
void _Gps_loop()  // run this in mail loop
{
  GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      _Gps_parse_flt = true;
    } else _Gps_parse_flt = false;
  }
  if (millis() - timer > 2000) {
    timer = millis();
    String dateString = "";
    String timeString = "";
    dateString += String(GPS.day, DEC);
    dateString += "/";
    dateString += String(GPS.month, DEC);
    dateString += "/";
    dateString += ("20");
    dateString += String(GPS.year, DEC);
    dateString += " ";
    if (GPS.hour < 10) { timeString += "0"; }
    timeString += String(GPS.hour, DEC);
    timeString += ":";
    if (GPS.minute < 10) { timeString += "0"; }
    timeString += String(GPS.minute, DEC);
    timeString += ":";
    if (GPS.seconds < 10) { timeString += "0"; }
    timeString += String(GPS.seconds, DEC);
    dateString += timeString;
    if (GPS.fix) {
      DynamicJsonDocument doc(512);
      doc["hbt"] = (millis() / 1000);
      doc["gpst"] = (int)GPS.fix;
      doc["Dt"] = dateString;
      //doc["Time"] = timeString;
      //doc["Quality"] = (int)GPS.fixquality;
      float lat = convertToDecimalDegrees(GPS.latitude);
      doc["lat"] = round(lat * 10000.0) / 10000.0;
      //doc["lat"] = String(GPS.lat);
      float lon = convertToDecimalDegrees(GPS.longitude);
      doc["lon"] = round(lon * 10000.0) / 10000.0;
      //doc["lon"] = String(GPS.lon);
      //doc["Spd"] = GPS.speed;
      //doc["Angle"] = GPS.angle;
      doc["ele"] = GPS.altitude;
      doc["Sat"] = (int)GPS.satellites;
      // doc["Ant"] = (int)GPS.antenna;
      serializeJson(doc, _Gps_Payload);
    } else {
      DynamicJsonDocument doc(512);
      doc["gpst"] = (int)GPS.fix;
      doc["Date"] = dateString;
      doc["Time"] = timeString;
      serializeJson(doc, _Gps_Payload);
    }
  }
}