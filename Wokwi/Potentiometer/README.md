# ESP32 HTTP Server Example

ESP32 web server example: control 2 LEDs from a web page hosted on the ESP32.

Use [Wokwi for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=wokwi.wokwi-vscode) to simulate this project.

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

Once the simulation is running, open http://localhost:8180 in your web browser to interact with the simulated HTTP server.
