https://github.com/thorsten-l/ESP32-HELTEC-LoRa-TTN-OTAA

# ESP32-HELTEC-LoRa-TTN-OTAA

Sample LoRaWAN code for *HELTEC WiFi LoRa V2* and *V3* boards using PlatformIO.

## TTN config data

After power on or hard reset (not after recovering from deep sleep) the board prints out on the serial line: 

```
LoRaWAN_APP Jan 15 2025 09:13:01
  HELTEC_BOARD=30
AppConfig loaded.

Magic: 19660304
Sleeptime: 60000ms

AppEUI: 0102030405060708
DevEUI: 0102030405060708
AppKey: 01020304050607080102030405060708
```

AppEUI, DevEUI, and AppKey will be randomly generated and permanently stored in the NVS Memory. Under normal circumstances, they will never change.

```
AppEUI/JoinEUI: 6A001E369F7E8CC0
        DevEUI: 86DB19F7127E9F15
        AppKey: 1ECC7CB7AD77A282170ABEE15DE92ADA


LoRaWAN EU868 Class A start!

+OTAA=1
+Class=A
+ADR=1
+IsTxConfirmed=1
+AppPort=2
+DutyCycle=1200000
+ConfirmedNbTrials=4
+ChMask=0000000000000000000000FF
+DevEui=86DB19F7127E9F15(For OTAA Mode)
+AppEui=6A001E369F7E8CC0(For OTAA Mode)
+AppKey=1ECC7CB7AD77A282170ABEE15DE92ADA(For OTAA Mode)
+NwkSKey=00000000000000000000000000000000(For ABP Mode)
+AppSKey=00000000000000000000000000000000(For ABP Mode)
+DevAddr=00000000(For ABP Mode)
```

Sleep time is 20min --> 1200000ms