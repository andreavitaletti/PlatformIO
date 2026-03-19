These projects are developed using [Platformio](https://platformio.org/)

I use the [Antigravity](https://antigravity.google/)  (or [VScode](https://code.visualstudio.com/)) as IDE

I installed a few plugins, among which the [WokWI simulator](https://wokwi.com/) 

I found it convenient to set up a virtual environment to run Platformio via the PlatformIO Core (CLI) as explained in the [Quick start](https://docs.platformio.org/en/latest/core/quickstart.html)

Set up the project by the [platformio.ini](https://docs.platformio.org/en/latest/projectconf/index.html)

Assuming the platformio.ini looks like the following: 

```
[env:common]
...
build_src_filter = 
	+<*.h> 
	-<.git/> 
	-<.svn/>
	+<shared/>
	+<${PIOENV}/>
	+<main-${PIOENV}.cpp>

[env:ADC]
...

[env:DAC]
...
```

```

# To just compile main-ADC.cpp in the src folder
pio run -e ADC

# To just compile main-DAC.cpp in the src folder
pio run -e DAC

# To compile and upload immediately
pio run -e DAC --target upload

# To compile, upload, and start the serial monitor
pio run -e DAC --target upload --target monitor

# I experienced some problems to synch the baudrate with the above, the followgin command fix it

platformio device monitor -b 115200 -p /dev/ttyUSB1

# Multiple devices connected
pio device list

# Upload to a specific device
pio run -e DAC -t upload --upload-port /dev/ttyUSB0

```

You can use the [Better Serial Plotter](https://hackaday.io/project/181686-better-serial-plotter) to plot the data acquired by the ADC	

If the signal is relatively slow, you can even use [Web Serial Plotter](https://github.com/atomic14/web-serial-plotter?tab=readme-ov-file)