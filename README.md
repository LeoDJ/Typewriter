# Typewriter
IoT controlled electrical typewriter

## File Structure
Folder | Component | Description
--- | --- | ---
Hardware | Teensy 3.2 | Used for interfacing the typewriter, receives text to type over Serial
Client | ESP8266 | Connects via Websocket to server and receives data to type
Server | node.js script | Interfaces between the Twitter API and Websocket connection
