# PickleballSwingTracker
**How To Use:**
1) Install PlatformIO extension for Visual Studio Code

2) Import/Open project

3) Upload code to the ESP32

**Data Testing Instructions:**

**NOTE: Make sure you uploaded the code to the ESP32 before opening putty sessoin or the upload will fail** 
1) Download PuTTY

2) Configure settings for UART communication: 460800 baud, 8 data bits, 1 stop, no parity, no flow control

![image](https://github.com/user-attachments/assets/f4a81242-550c-444d-b66f-b15a0a70aa4b)
![image](https://github.com/user-attachments/assets/c32ee763-2266-43b2-bfcb-b3f892bcd0c7)

3) Configure putty logging: all session output, output putty.log to repo's test directory, always overwite it

![image](https://github.com/user-attachments/assets/12c5226d-11f0-4aad-9467-55ef8ef6a9f5)

4) Hold push button to start/stop data dump

5) Close PuTTY terminal to save/update log

6) Navigate to test folder and run data_testing.py

**NOTE: If you do not already have matplotlib installed, you will need to install it with 'pip install matplotlib'** 

**Known Bugs:**
- Occasional bounce of push button upon release (can be fixed with debounce circuitry)
- High speed UART communication to dump data sometimes sends misaligned data (close other appliations or restart computer to solve)

**Current Breadboard Setup:**

![IMG_1425](https://github.com/user-attachments/assets/c9d3cb88-8666-49c8-98ce-e84242278bd4)

**Completed Work:**
- Project Setup: setup project using platform.io and confirmed the ability to build and upload code to the ESP32 microcontroller.
- UART Communication: created a UART driver for the ESP32 and established communication between the ESP32 and our computers.
- SPI Communication: created an SPI driver for the ESP32 and established communication between ESP32 and our IMU.
- FreeRTOS: Integrated FreeRTOS into our project and created multiple threads that can run simulataneously.
- BLE: Created BLE driver using nimBLE. Can connect to a client device and send a message upon read.
- SPIFFS: Set up file storage system to be used in the future.
- Button: Created button driver that can detect number of presses and holds.
- Data Analysis: Using a python script and matplotlib, we have plots of raw sensor data and data with various filters.
  
**Project Architecture:**

The project architecture is straightforward at this point. We have header and source files for the drivers we have completed so far, and are leveraging various drivers that platform.io provides. The directory structure is as follows: 
- include: houses the header files for SPI, UART, threads, the LSM6DSL, and will house any future header files. 
- lib: will house any libraries that we include in our project.
- src: houses the source code for SPI, UART, threads, the LSM6DSL, our main.c, and will house any future source code. 
- test: will include any testing files we create. 
