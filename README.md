# PickleballSwingTracker
**How To Use:**
1) Install PlatformIO extension for Visual Studio Code
2) Import/Open project
3) Upload code to the ESP32
4) Wait for start up sequence to finish (enter idle state)
5) Single tap button for new play session
6) Play pickleball
7) Single tap button again to end play session (re-enter idle state)
8) Double tap button to start bluetooth session
9) Once app is paired, dump and view data
10) Hold button to reset device and delete all data (optional)

**LED States:**
1) White blink: Start-up sequence
2) Solid white: Idle
3) Solid green/red: Active play session and battery life indicator
4) Blue blink: BLE awaiting connection
5) Solid blue: BLE connected
6) Blue blink (rapid): Data dump in progress

**Known Bugs (as of 2/20/25):**
- Impacts are sometimes ignored
- Inaccuracy of "max rotation" data (Gyroscope too sensitive; Value always clipped)
- BLE session end while client reading occasionally causes crash
  
**Current Setup (as of 2/20/25):**

<img src="https://github.com/user-attachments/assets/ddcb1e78-f5e6-4834-baf4-70b0cd143ea0" width="270" />
<img src="https://github.com/user-attachments/assets/6a9e5996-adc3-4e18-b5b7-70eb57418827" width="270" /> <br />

*Battery not yet integrated into design

**Completed Work (as of 2/20/25):**
- UART, SPI, FreeRTOS, BLE, SPIFFS, custom circular buffer integration
- Button gesture detection
- State machine to start/end play and bluetooth sessions or reset device
- RGB LED to indicate device state
- Parsing of sensor data for relevant points to send to mobile application
- Capability to dump data to serial plot for viewing
  
**Project Architecture (as of 2/20/25):**

The project architecture is straightforward. Platform.io links various libraries/the ESP-IDF to the project. 
The directory structure is as follows: 
- include: all header files - drivers, circular buffer, play session, FSM, bluetooth session (in BLE driver). 
- src: all c files corresponding to their headers and main. 
