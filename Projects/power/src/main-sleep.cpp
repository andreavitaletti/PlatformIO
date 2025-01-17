#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "secrets.h"
/*
  ESP32 Deep Sleep Mode Timer Wake UP
 http:://www.electronicwings.com
*/ 
// mosquitto_pub -h test.mosquitto.org -t "avitaletti/feeds/threshold" -m 2345
// mosquitto_sub -h test.mosquitto.org -t "avitaletti/feeds/potentiometer"


#define Time_To_Sleep 5   //Time ESP32 will go to sleep (in seconds)
#define S_To_uS_Factor 1000000ULL      //Conversion factor for micro seconds to seconds 

RTC_DATA_ATTR int bootCount= 0;

const char* mqttServer = "test.mosquitto.org";
int port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

int threshold = 2000;
long lastMsg = 0;
int val = 0;
int result = 0;
int wificounter = 0;
char clientId[50];

void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void mqttReconnect();

void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  setup_wifi();
  
  client.setServer(mqttServer, port);
  client.setCallback(callback);

  if (!client.connected()) {
    mqttReconnect();
  }
  // checks for new messages
  client.loop();

   /*
    SENSE
    e.g. val = analogRead(potPin); 
    */
    val = random(4000);
    snprintf (msg, MSG_BUFFER_SIZE, "%ld", val);
    Serial.print("Publish message: ");
    Serial.println(msg);
    result = client.publish("avitaletti/feeds/potentiometer", msg);
    Serial.println(result);

  //Set timer to 5 seconds
 esp_sleep_enable_timer_wakeup(Time_To_Sleep * S_To_uS_Factor);
  Serial.println("Setup ESP32 to sleep for every " + String(Time_To_Sleep) +
  " Seconds");

  //Go to sleep now
  esp_deep_sleep_start();
 
  Serial.println("This will not print!!"); // This will not get print,as ESP32 goes in Sleep mode.
}

void loop() {} // We don't need loop as ESP32 will initilize each time.


void setup_wifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.persistent(false);
        WiFi.mode(WIFI_OFF);
        WiFi.mode(WIFI_STA);

        WiFi.begin(ssid, password);
    }

    wificounter = 0;
    while (WiFi.status() != WL_CONNECTED && wificounter < 10)
    {
        for (int i = 0; i < 500; i++)
        {
            delay(1);
        }
        Serial.print(".");
        wificounter++;
    }

    if (wificounter >= 10)
    {
        Serial.println("Restarting ...");
        ESP.restart(); //targetting 8266 & Esp32 - you may need to replace this
    }

    delay(10);

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void mqttReconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    long r = random(1000);
    sprintf(clientId, "clientId-%ld", r);
    // to connect to adafruit
    // if (client.connect(clientId,SECRET_MQTT_USER, SECRET_MQTT_API_KEY)) {
    if (client.connect(clientId)) {
      Serial.print(clientId);
      Serial.println(" connected");
      //client.subscribe("avitaletti/feeds/threshold");
      // if subscribe give some time
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String stMessage;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    stMessage += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "avitaletti/feeds/threshold") {
    Serial.print("New Threshold is: ");
    Serial.println(stMessage);
    threshold = stMessage.toInt();
  }
}