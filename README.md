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

NOTE: You can record multiple play sessions by repeating steps 5-7 before dumping. If you run out of storage or hit the max number of play sessions, you will need to dump to continue. 

**LED States:**
1) White blink: Start-up sequence
2) Solid white: Idle
3) Solid green/red: Active play session and battery life indicator
4) Blue blink: BLE awaiting connection
5) Solid blue: BLE connected
6) Blue blink (rapid): Data dump in progress
7) Yellow blink: Storage full or max sessions reached
  
**Current Setup (as of 4/16/25):**

<img src="https://github.com/user-attachments/assets/2bb0c313-fce6-4b20-83d0-c53e85ee2064" width="270" />
<img src="https://github.com/user-attachments/assets/8af5b6f0-d76a-4a72-b480-cc793eb54ec3" width="270" /> <br />
<img src="https://github.com/user-attachments/assets/05e0207b-de0d-4a45-b1a6-82cd4885a2a9" width="270" />
<img src="https://github.com/user-attachments/assets/82d11c31-6d55-4d73-808e-f44530e34c87" width="270" /> <br />
