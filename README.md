# PickleballSwingTracker
Completed Work:
- Project Setup: setup project using platform.io and confirmed the ability to build and upload code to the ESP32 microcontroller.
- UART Communication: created a UART driver for the ESP32 and established communication between the ESP32 and our computers.
- SPI Communication: created an SPI driver for the ESP32 and established communication between the ESP32 and LSM6DSL by reading the LSM6DSL's WHO_AM_I register and verifying its output.
- FreeRTOS: Integrated FreeRTOS into our project and created multiple threads that can run simulataneously. 
  
Project Architecture:
The project architecture is straightforward at this point. We have header and source files for the drivers we have completed so far, and are leveraging various drivers that platform.io provides. The directory structure is as follows: 
- include: houses the header files for SPI, UART, threads, the LSM6DSL, and will house any future header files. 
- lib: will house any libraries that we include in our project.
- src: houses the source code for SPI, UART, threads, the LSM6DSL, our main.c, and will house any future source code. 
- test: will include any testing files we create. 

Known Bugs:
- There are currently no known bugs. We do get a watchdog warning that we are looking into, but SPI, UART, and threading are operating as expected to our knowledge. 
