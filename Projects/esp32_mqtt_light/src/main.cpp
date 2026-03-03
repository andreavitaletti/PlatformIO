#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// --------------------------------------------------------------------------
// Configuration: USER MUST UPDATE THESE
// --------------------------------------------------------------------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_BROKER_IP";
const int mqtt_port = 1883;

// --------------------------------------------------------------------------
// Pin Definitions
// --------------------------------------------------------------------------
// Connect your Analog Light Sensor (LDR) to this pin
const int LDR_PIN = 34; 

// --------------------------------------------------------------------------
// Topics
// --------------------------------------------------------------------------
const char* TOPIC_LIGHT_VALUE = "sensors/light/value";
const char* TOPIC_THRESHOLD_SET = "sensors/light/threshold/set";
const char* TOPIC_STATUS = "sensors/light/status";

// --------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------
WiFiClient espClient;
PubSubClient client(espClient);

// Default threshold, can be updated via MQTT
int lightThreshold = 2000; 

// Timing for non-blocking loop
unsigned long lastMsgTime = 0;
const long interval = 2000; // Check sensor every 2 seconds

// Function Prototypes
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
  Serial.begin(115200);
  delay(100);

  setup_wifi();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Configure Analog Pin
  pinMode(LDR_PIN, INPUT);
  
  Serial.println("Setup complete.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsgTime > interval) {
    lastMsgTime = now;
    
    // Read Light Value (0-4095 for ESP32 12-bit ADC)
    int lightValue = analogRead(LDR_PIN);
    
    Serial.print("Light Level: ");
    Serial.print(lightValue);
    Serial.print(" | Threshold: ");
    Serial.println(lightThreshold);

    // Only publish if above threshold
    if (lightValue > lightThreshold) {
      char msg[50];
      snprintf(msg, 50, "%d", lightValue);
      Serial.print("Publishing message: ");
      Serial.println(msg);
      client.publish(TOPIC_LIGHT_VALUE, msg);
    }
  }
}

// --------------------------------------------------------------------------
// Wifi Setup
// --------------------------------------------------------------------------
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// --------------------------------------------------------------------------
// MQTT Message Callback
// --------------------------------------------------------------------------
// This function is executed when a message arrives on a subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Create a temporary buffer for the payload to safely null-terminate/parse
  char message[length + 1];
  for (int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  Serial.println(message);

  // Check if the message is for the threshold topic
  if (String(topic) == TOPIC_THRESHOLD_SET) {
    int newThreshold = atoi(message);
    // Simple validation (0-4095 is max range for ADC)
    if (newThreshold >= 0 && newThreshold <= 4095) {
        lightThreshold = newThreshold;
        Serial.print("New threshold set to: ");
        Serial.println(lightThreshold);
        
        // Optional: Acknowledge the change
        char status_msg[64];
        snprintf(status_msg, 64, "Threshold updated to %d", lightThreshold);
        client.publish(TOPIC_STATUS, status_msg);
    } else {
        Serial.println("Invalid threshold value received.");
    }
  }
}

// --------------------------------------------------------------------------
// Reconnect Loop
// --------------------------------------------------------------------------
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(TOPIC_STATUS, "ESP32 Light Sensor Online");
      // ... and resubscribe
      client.subscribe(TOPIC_THRESHOLD_SET);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
