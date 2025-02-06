# PickleballSwingTracker
**How To Use:**
1) Install PlatformIO extension for Visual Studio Code

2) Import/Open project

3) Upload code to the ESP32

**Known Bugs (as of 12/6/24):**
- Occasional bounce of push button upon release (can be fixed with debounce circuitry)
- High speed UART communication to dump data sometimes sends misaligned data (close other appliations or restart computer to solve)

**Current Breadboard Setup (as of 12/6/24):**

![IMG_1425](https://github.com/user-attachments/assets/c9d3cb88-8666-49c8-98ce-e84242278bd4)

**Completed Work (as of 12/6/24):**
- Project Setup: setup project using platform.io and confirmed the ability to build and upload code to the ESP32 microcontroller.
- UART Communication: created a UART driver for the ESP32 and established communication between the ESP32 and our computers.
- SPI Communication: created an SPI driver for the ESP32 and established communication between ESP32 and our IMU.
- FreeRTOS: Integrated FreeRTOS into our project and created multiple threads that can run simulataneously.
- BLE: Created BLE driver using nimBLE. Can connect to a client device and send a message upon read.
- SPIFFS: Set up file storage system to be used in the future.
- Button: Created button driver that can detect number of presses and holds.
- Data Analysis: Using a python script and matplotlib, we have plots of raw sensor data and data with various filters.
  
**Project Architecture (as of 12/6/24):**

The project architecture is straightforward at this point. We have header and source files for the drivers we have completed so far, and are leveraging various drivers that platform.io provides. The directory structure is as follows: 
- include: houses the header files for SPI, UART, threads, the LSM6DSL, and will house any future header files. 
- lib: will house any libraries that we include in our project.
- src: houses the source code for SPI, UART, threads, the LSM6DSL, our main.c, and will house any future source code. 
- test: will include any testing files we create. 
