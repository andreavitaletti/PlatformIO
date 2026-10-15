# Simple ADC on Arduino and FreeRTOS

## Building

This is a [PlatformIO](https://platformio.org) project. To build it, [install PlatformIO](https://docs.platformio.org/en/latest/core/installation/index.html), and then run the following command:

```
# To just compile
pio run -e potentiometer_cpp

# To compile and upload immediately
pio run -e potentiometer_cpp --target upload

# To compile, upload, and start the serial monitor
pio run -e potentiometer_cpp --target upload --target monitor

```

## Simulating

Change the wokwi.toml 

To simulate this project, install [Wokwi for VS Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode). Open the project directory in Visual Studio Code, press **F1** and select "Wokwi: Start Simulator".

