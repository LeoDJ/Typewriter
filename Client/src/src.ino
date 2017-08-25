/* Used libraries:
 * https://github.com/esp8266/Arduino
 * https://github.com/Links2004/arduinoWebSockets
 * https://github.com/bblanchon/ArduinoJson
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WebSocketsClient.h>

#include <Hash.h>

#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

const byte ledPin = D0;

#define TTY_SERIAL Serial
#define USE_SERIAL Serial1

#define MESSAGE_INTERVAL 30000
#define HEARTBEAT_INTERVAL 25000

uint64_t messageTimestamp = 0;
uint64_t heartbeatTimestamp = 0;
bool isConnected = false;

StaticJsonBuffer<1024> jsonBuffer;

String result = "";

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            isConnected = false;
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
                isConnected = true;

			    // send message to server when Connected
                // socket.io upgrade confirmation message (required)
				webSocket.sendTXT("5");
            }
            break;
        case WStype_TEXT: {
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            String payloadStr = (char*)payload;
            if(payloadStr.startsWith("42[")) {
                payloadStr.remove(0,2); //remove first 2 chars ("42")
                jsonBuffer = StaticJsonBuffer<1024>();
                JsonArray& array = jsonBuffer.parseArray(payloadStr);

                // Test if parsing succeeds.
                if (!array.success()) {
                    USE_SERIAL.printf("parseArray() failed");
                    return;
                }
                /*JsonArray::iterator it = root.begin();
                String messageType = it -> as<char*>();*/
                if(array[0] == "printTweet") {
                    String jsonString = array[1];
                    //jsonString.replace("\\", "");
                    JsonObject& root = jsonBuffer.parseObject(jsonString);
                    if (!root.success()) {
                        USE_SERIAL.printf("parseObject() failed");
                        return;
                    }
                    //Serial.println((const char*)root["text"]);
                    result = "(@";
                    result += (const char*)root["userName"];
                    result += ") ";
                    result += (const char*)root["text"];

                }

                if(array[0] == "printText") {
                    String jsonString = array[1];
                    //jsonString.replace("\\", "");
                    JsonObject& root = jsonBuffer.parseObject(jsonString);
                    if (!root.success()) {
                        USE_SERIAL.printf("parseObject() failed");
                        return;
                    }
                    result = (const char*)root["text"];
                }
            }
			// send message to server
			// webSocket.sendTXT("message here");
            } break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }
    digitalWrite(ledPin, !isConnected); //led is active low
}

void setup() {
    // USE_SERIAL.begin(921600);
    TTY_SERIAL.begin(115200);
    USE_SERIAL.begin(115200);

    //Serial.setDebugOutput(true);
    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("FABLAB", "handfulofrain");
    WiFiMulti.addAP("Freifunk", "");
    WiFiMulti.addAP("CoworkingHN", "ilovecowohn");

    //WiFi.disconnect();
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    //webSocket.beginSocketIO("192.168.178.242", 8001);
    webSocket.beginSocketIO("gingerlabs.de", 58001);
    //webSocket.beginSSL("192.168.178.74", 8001, "/socket.io/?EIO=3");
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);
    pinMode(ledPin, OUTPUT);
}

void loop() {
    webSocket.loop();

    //error handling (too many messages, etc)
    if(result.length() > 0) {
        TTY_SERIAL.println(result);
        result = "";
    }

    if(TTY_SERIAL.available()) {
        char c = TTY_SERIAL.read();
        if(c == '\x06');
            webSocket.sendTXT("42[\"printDone\",{\"success:\":true}]");
            //send ACK back
    }

    /*if(USE_SERIAL.available())
      TTY_SERIAL.print((char)USE_SERIAL.read());

    if(TTY_SERIAL.available())
      TTY_SERIAL.print((char)TTY_SERIAL.read());*/

    if(isConnected) {

        uint64_t now = millis();

        /*if(now - messageTimestamp > MESSAGE_INTERVAL) {
            messageTimestamp = now;
            // example socket.io message with type "messageType" and JSON payload
            webSocket.sendTXT("42[\"messageType\",{\"greeting\":\"hello\"}]");
        }*/

        if((now - heartbeatTimestamp) > HEARTBEAT_INTERVAL) {
            heartbeatTimestamp = now;
            // socket.io heartbeat message
            webSocket.sendTXT("2");
        }
    }
}
