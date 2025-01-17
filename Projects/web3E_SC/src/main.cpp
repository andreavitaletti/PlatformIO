#include <WiFi.h>
#include <Web3.h>
// capital Web3.h as well as Trezor.h
#include <Util.h>
#include <Contract.h>
#include "secrets.h"
#include <iostream>
#include <DHT.h>

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
// #define TIME_TO_SLEEP  300        /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_SLEEP  300 


RTC_DATA_ATTR int bootCount = 0;

int wificounter = 0;
Web3 *web3;

void setup_wifi();
void callContract(const char *destination, float t_dht, float h_dht); 
void FloatToHex(float f, byte* hex);

#define DHT22_PIN  4 // ESP32 pin GPIO4 connected to DHT22 sensor
DHT dht22(DHT22_PIN, DHT22);

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("Wakeup caused by ULP program"); break;
    default:                        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}



void setup() {

    Serial.begin(115200);
    delay(1000);  //Take some time to open up the Serial Monitor
    Serial.print("start setup:");
    Serial.println(millis());

    /* in nodes.h 
    add:
    #define SEPOLIA_INF_RPC_URL "sepolia.infura.io/v3/" 
    
    change: 
    case SEPOLIA_ID:
        // return SEPOLIA_URL;
        return SEPOLIA_INF_RPC_URL;

    in certificates.h
    
    change:
        case SEPOLIA_ID:
        // return sepolia_cert;
        return infura_ca_cert;    
    */

    web3 = new Web3(SEPOLIA_ID);

    setup_wifi();

    dht22.begin(); 

      //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  //Print the wakeup reason for ESP32
  print_wakeup_reason();

    Serial.print("end setup:");
    Serial.println(millis());
    
    Serial.print("start DHT reading:");
    Serial.println(millis());

    float t_dht =  dht22.readTemperature();
    float h_dht = dht22.readHumidity();

    Serial.print("end DHT reading:");
    Serial.println(millis());

    callContract(CONTRACTADDRESS,t_dht,h_dht);

     esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
     Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
    Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");

 
}

void FloatToHex(float f, byte* hex){
  byte* f_byte = reinterpret_cast<byte*>(&f);
  memcpy(hex, f_byte, 4);
}

void callContract(const char *contractAddr, float t_dht, float h_dht)
{

    Serial.print("start call contract:");
    Serial.println(millis());
  
  //obtain a contract object, for just sending eth doesn't need a contract address
  string contractAddrStr = contractAddr;
	Contract contract(web3, contractAddr);
	contract.SetPrivateKey(PRIVATE_KEY);
	unsigned long long gasPriceVal = 22000000000ULL;
	uint32_t  gasLimitVal = 90000;

	Serial.print("Get Nonce: ");
    string address = MY_ADDRESS;
	uint32_t nonceVal = (uint32_t)web3->EthGetTransactionCount(&address);
	Serial.println(nonceVal);

  uint256_t callValue = 0;
  //uint8_t dataStr[100];
  //memset(dataStr, 0, 100);
  //string p = contract.SetupContractData((char*)dataStr, "send_data(uint,uint)", 18, 31);
  //string p = contract.SetupContractData("send_data(uint,uint)", 18, 31);
  // method 0xf89f0dfd <--  send_data(uint, uint) 
  // 0000000000000000000000000000000000000000000000000000000000000015 in HEX ... UINT256 --> 32byte
  // 0000000000000000000000000000000000000000000000000000000000000021 in HEX



  //string method = "0xf89f0dfd"; //SC store
  string method = "0x23972aef"; //SC lottery
  //int temp_encoded = 40 + static_cast<int>(round(t_dht));
  int temp_encoded = 20;
  char hex_temp[64];
  sprintf(hex_temp, "%064X", temp_encoded);
  string temp_string = hex_temp;
  
  // int hum = static_cast<int>(round(h_dht));
  int hum = 15;
  char hex_hum[64];
  sprintf(hex_hum, "%064X", hum);
  string hum_string = hex_hum;

  if ( isnan(h_dht) || isnan(t_dht)) {
    Serial.println("Failed to read from DHT22 sensor!");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h_dht);
    Serial.print("%");

    Serial.print("  |  ");

    Serial.print("Temperature: ");
    Serial.print(t_dht);
    Serial.print("°C  ");
  }  


  string p = method+temp_string+hum_string;   
  //string p = "0xf89f0dfd00000000000000000000000000000000000000000000000000000000000000150000000000000000000000000000000000000000000000000000000000000021";
// send transaction

string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &contractAddrStr, &callValue, &p);
Serial.println(result.c_str());
  string transactionHash = web3->getResult(&result);

Serial.println("TX on Etherscan:");
  Serial.print(ETHERSCAN_TX);
  Serial.print("/0x");
  Serial.println(transactionHash.c_str()); 


    Serial.print("end call contract:");
    Serial.println(millis());

}



void loop() {

}


/* This routine is specifically geared for ESP32 perculiarities */
/* You may need to change the code as required */
/* It should work on 8266 as well */
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