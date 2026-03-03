These projects are developed using [Platformio](https://platformio.org/)

I use the [Antigravity](https://antigravity.google/)  (or [VScode](https://code.visualstudio.com/)) as IDE

I installed a few plugins, among which the [WokWI simulator](https://wokwi.com/) 

I found it convenient to set up a virtual environment to run Platformio via the PlatformIO Core (CLI) as explained in the [Quick start](https://docs.platformio.org/en/latest/core/quickstart.html)

Set up the project by the [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html) 

```
pio run -t upload -e espwroom32

platformio device monitor -b 115200

```
