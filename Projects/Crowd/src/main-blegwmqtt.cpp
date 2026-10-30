#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --------------------------------------------------
// Configuration
// --------------------------------------------------

#define DEVICE_NAME "ESP32-Sensor"

#define SERVICE_UUID \
  "12345678-1234-1234-1234-123456789001"

#define CHARACTERISTIC_UUID \
  "12345678-1234-1234-1234-123456789002"

const char* DEVICE_ID = "esp32-001";

const uint32_t TRANSMISSION_INTERVAL_MS = 5000;

// --------------------------------------------------
// BLE objects and state
// --------------------------------------------------

BLEServer* bleServer = nullptr;
BLECharacteristic* sensorCharacteristic = nullptr;

bool deviceConnected = false;

uint32_t sequenceNumber = 0;
uint32_t lastTransmission = 0;

// --------------------------------------------------
// BLE connection callbacks
// --------------------------------------------------

class ServerCallbacks : public BLEServerCallbacks {

  void onConnect(BLEServer* server) override {
    deviceConnected = true;
    Serial.println("Smartphone connected!");
  }

  void onDisconnect(BLEServer* server) override {
    deviceConnected = false;
    Serial.println("Smartphone disconnected!");

    // Restart advertising so the phone can reconnect
    BLEDevice::startAdvertising();
  }
};

// --------------------------------------------------
// Sensor readings
// Replace these functions with your actual sensors
// --------------------------------------------------

float readTemperature() {
  // TODO: Replace with actual temperature sensor reading
  return 23.5;
}

float readHumidity() {
  // TODO: Replace with actual humidity sensor reading
  return 55.0;
}

// --------------------------------------------------
// BLE initialization
// --------------------------------------------------

void initializeBLE() {

  Serial.println("Initializing BLE...");

  BLEDevice::init(DEVICE_NAME);

  // Request a larger ATT MTU for JSON notifications.
  // The effective MTU depends on what the phone negotiates.
  BLEDevice::setMTU(185);

  // Create BLE server
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(new ServerCallbacks());

  // Create BLE service
  BLEService* service = bleServer->createService(SERVICE_UUID);

  // Create characteristic
  sensorCharacteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  // Enable notifications on the client side
  sensorCharacteristic->addDescriptor(new BLE2902());

  // Start service
  service->start();

  // Configure advertising
  BLEAdvertising* advertising = BLEDevice::getAdvertising();

  advertising->addServiceUUID(SERVICE_UUID);
  advertising->setScanResponse(true);

  // Start advertising
  advertising->start();

  Serial.println("BLE initialized.");
  Serial.println("Waiting for smartphone connection...");
}

// --------------------------------------------------
// Transmit sensor measurements
// --------------------------------------------------

void transmitMeasurements() {

  float temperature = readTemperature();
  float humidity = readHumidity();

  uint32_t uptime = millis();

  char payload[180];

  snprintf(
    payload,
    sizeof(payload),
    "{"
      "\"id\":\"%s\","
      "\"seq\":%lu,"
      "\"temp\":%.2f,"
      "\"humidity\":%.2f,"
      "\"uptime_ms\":%lu"
    "}",
    DEVICE_ID,
    (unsigned long)sequenceNumber++,
    temperature,
    humidity,
    (unsigned long)uptime
  );

  // Update characteristic value
  sensorCharacteristic->setValue(
    (uint8_t*)payload,
    strlen(payload)
  );

  // Notify connected smartphone
  sensorCharacteristic->notify();

  Serial.print("Sent: ");
  Serial.println(payload);
}

// --------------------------------------------------
// Arduino setup
// --------------------------------------------------

void setup() {

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("ESP32 Sensor Gateway");
  Serial.println("--------------------");

  initializeBLE();
}

// --------------------------------------------------
// Arduino main loop
// --------------------------------------------------

void loop() {

  uint32_t now = millis();

  if (deviceConnected &&
      now - lastTransmission >= TRANSMISSION_INTERVAL_MS) {

    lastTransmission = now;

    transmitMeasurements();
  }

  delay(10);
}
