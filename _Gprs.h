#include <PubSubClient.h>
#define TINY_GSM_MODEM_SIM7600
// #define SerialMon Serial
// #define TINY_GSM_DEBUG SerialMon
#define TINY_GSM_USE_GPRS true
#define GSM_PIN ""
bool _Gprs_network_sts = false;
bool _Gprs_gprs_sts = false;
int _Gprs_reconnect_count = 0;
bool _Gprs_mqtt_sts = false;
//Serial Defination
#define _Gprs_RX 21
#define _Gprs_TX 22
#define _Gprs_serial_Baud 115200
HardwareSerial _Gprs_serial(2);
#define MQTT_KEEPALIVE 60

const char apn[] = "jionet";
const char gprsUser[] = "";
const char gprsPass[] = "";

const char* broker = "io.adafruit.com";
const char* topicLed = "Ashish_82848284/feeds/location";
const char* topicSub = "Ashish_82848284/feeds/abc";
unsigned long lastmillies = 0;
unsigned long publishdelay = 5000;
unsigned long onesec_elspd = 0;
uint32_t lastReconnectAttempt = 0;

#include <TinyGsmClient.h>

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

TinyGsm modem(_Gprs_serial);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

void mqtt_pub_call() {
  if (millis() - lastmillies > publishdelay) {
    lastmillies = millis();
    if (_Gprs_mqtt_sts) mqtt.publish(topicLed, _Gps_Payload.c_str());
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

boolean mqttConnect() {
  boolean status = mqtt.connect("GsmClientName", "Ashish_82848284", "82184014511d457ca7c0775480419bb2");
  if (status == false) {
    return false;
  }
  mqtt.subscribe(topicSub);
  return mqtt.connected();
}
void _Gprs_initialize() {
  // SerialMon.println("Initializing modem...");
  if (_Gprs_reconnect_count < 1) modem.init();
  else modem.restart();
  String modemInfo = modem.getModemInfo();
  // SerialMon.print("Modem Info: ");
  // SerialMon.println(modemInfo);
  if (GSM_PIN && modem.getSimStatus() != 3) {
    modem.simUnlock(GSM_PIN);
  }
  // SerialMon.print(F("Connecting to "));
  // SerialMon.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    // SerialMon.println(" fail");
    delay(5000);
    return;
  }
  // SerialMon.println(" success");
  if (modem.isGprsConnected()) {
    // SerialMon.println("GPRS connected");
  }
}

void _Gprs_setup() {
  _Gprs_serial.begin(_Gprs_serial_Baud, SERIAL_8N1, _Gprs_RX, _Gprs_TX);
  delay(1000);
  _Gprs_initialize();
  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  lastmillies = millis();
  onesec_elspd = millis();
}

void _Gprs_loop() {
  if (_Gprs_reconnect_count >= 1) {
    _Gprs_initialize();
    _Gprs_reconnect_count = 0;
  }
  // Network Checking
  if (!modem.isNetworkConnected()) {
    _Gprs_network_sts = false;
    _Gprs_gprs_sts = false;
    _Gprs_mqtt_sts = false;
    _Gprs_reconnect_count += 1;
    if (!modem.waitForNetwork(20000L, true)) {
      // SerialMon.println(" fail");
    }
  }
  // if Network Connected return if connected for mqtt checking
  if (modem.isNetworkConnected()) {
    _Gprs_network_sts = true;
    _Gprs_reconnect_count = 0;
    // Gprs Checking
    if (!modem.isGprsConnected()) {
      _Gprs_gprs_sts = false;
      _Gprs_mqtt_sts = false;
      _Gprs_reconnect_count += 1;
      if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        // SerialMon.println(" fail");
      }
    }
    if (modem.isGprsConnected()) {
      _Gprs_gprs_sts = true;
      _Gprs_reconnect_count = 0;
    }
  }
  // Mqtt checking if gprs is connected and loop mqtt if its connected
  if (_Gprs_gprs_sts) {
    if (!mqtt.connected()) {
      _Gprs_mqtt_sts = false;
      uint32_t t = millis();
      if (t - lastReconnectAttempt > 10000L) {
        lastReconnectAttempt = t;
        if (mqttConnect()) {
          lastReconnectAttempt = 0;
        }
      }
      delay(100);
    } else {
      _Gprs_mqtt_sts = true;
      mqtt.loop();
      mqtt_pub_call();
    }
  }
}