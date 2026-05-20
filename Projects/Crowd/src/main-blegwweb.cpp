/**
 * ESP32 BLE Sensor Node
 * =====================
 * Exposes a BLE GATT service with two characteristics:
 *   - SENSOR_CHAR  (notify)  : JSON payload of sensor readings
 *   - CMD_CHAR     (write)   : receive commands from the phone (optional)
 *
 * Replace the readSensors() stub with your real DHT22 / other sensor code.
 *
 * Dependencies (Arduino Library Manager):
 *   - "ESP32 BLE Arduino" by Neil Kolban (bundled with ESP32 board package)
 *
 * Board: ESP32 Dev Module
 */
#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

// ── UUIDs ────────────────────────────────────────────────────────────────────
// Generate fresh ones at https://www.uuidgenerator.net/  (must match the web app)
#define SERVICE_UUID        "12345678-1234-1234-1234-1234567890ab"
#define SENSOR_CHAR_UUID    "12345678-1234-1234-1234-1234567890ac"  // notify
#define CMD_CHAR_UUID       "12345678-1234-1234-1234-1234567890ad"  // write

// ── Config ───────────────────────────────────────────────────────────────────
#define DEVICE_NAME         "CSS-SensorNode"
#define SAMPLE_INTERVAL_MS  5000   // send a reading every 5 s

// ── Globals ──────────────────────────────────────────────────────────────────
BLECharacteristic *pSensorChar;
bool deviceConnected = false;
unsigned long lastSample = 0;

// ── Connection callbacks ──────────────────────────────────────────────────────
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("[BLE] Client connected");
  }
  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("[BLE] Client disconnected — restarting advertising");
    pServer->startAdvertising();
  }
};

// ── Command characteristic callbacks ─────────────────────────────────────────
class CmdCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string cmd = pChar->getValue();
    Serial.print("[CMD] Received: ");
    Serial.println(cmd.c_str());
    // e.g. "INTERVAL:10000" to change sample rate
  }
};

// ── Sensor stub ───────────────────────────────────────────────────────────────
// Replace with real DHT22 / BME280 / etc. readings
struct SensorData {
  float temperature;
  float humidity;
  long  timestamp_ms;
};

SensorData readSensors() {
  // TODO: replace with actual sensor code, e.g.:
  //   temperature = dht.readTemperature();
  //   humidity    = dht.readHumidity();
  return {
    .temperature  = -18.0 + random(-10, 10) * 0.1f,   // simulated freezer ≈ -18°C
    .humidity     = 15.0  + random(-5,  5)  * 0.1f,   // simulated 15% RH
    .timestamp_ms = millis()
  };
}

// ── Build JSON payload ────────────────────────────────────────────────────────
String buildJson(const SensorData &d) {
  // Keep it compact — BLE MTU is typically 20–512 bytes
  String json = "{";
  json += "\"t\":"  + String(d.temperature, 2) + ",";
  json += "\"h\":"  + String(d.humidity,    2) + ",";
  json += "\"ms\":" + String(d.timestamp_ms);
  json += "}";
  return json;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("[BLE] Initialising…");

  BLEDevice::init(DEVICE_NAME);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Sensor characteristic — notify
  pSensorChar = pService->createCharacteristic(
    SENSOR_CHAR_UUID,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pSensorChar->addDescriptor(new BLE2902());   // enables notifications

  // Command characteristic — write
  BLECharacteristic *pCmdChar = pService->createCharacteristic(
    CMD_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pCmdChar->setCallbacks(new CmdCallbacks());

  pService->start();

  BLEAdvertising *pAdv = BLEDevice::getAdvertising();
  pAdv->addServiceUUID(SERVICE_UUID);
  pAdv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("[BLE] Advertising as: " DEVICE_NAME);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  if (deviceConnected && millis() - lastSample >= SAMPLE_INTERVAL_MS) {
    lastSample = millis();

    SensorData data = readSensors();
    String payload   = buildJson(data);

    pSensorChar->setValue(payload.c_str());
    pSensorChar->notify();

    Serial.print("[BLE] Notified: ");
    Serial.println(payload);
  }
  delay(10);
}
