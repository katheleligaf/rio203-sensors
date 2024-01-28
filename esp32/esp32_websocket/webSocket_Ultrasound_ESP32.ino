#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
//#include <WebSocketClient.h>
#include "secret.h"
#include <math.h>

using namespace websockets;

#define echoPin 2
#define trigPin 4
const int ledRedPin = 23;
const int ledGreenPin = 19;
int idPlace = -1;

const String serverName = "parking-rio.rezel.net";
char path[] = "/";
char host[] = "parking-rio.rezel.net";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

const long soundSpeed = 343.2;
long duration, distance;

String previousState = "busy";
String currentState = "free";

const char ssl_cert[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n"
    "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
    "DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n"
    "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
    "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n"
    "AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n"
    "ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n"
    "wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n"
    "LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n"
    "4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n"
    "bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n"
    "sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n"
    "Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n"
    "FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n"
    "SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n"
    "PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n"
    "TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n"
    "SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n"
    "c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n"
    "+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n"
    "ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n"
    "b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n"
    "U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n"
    "MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n"
    "5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n"
    "9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n"
    "WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n"
    "he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n"
    "Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n"
    "-----END CERTIFICATE-----\n";

WiFiClientSecure client;
WebsocketsClient webSocketClient;

String getMacAddress() {
  byte mac[6]; // the MAC address of your
  WiFi.macAddress(mac);
  return String(mac[0], HEX) + ":" + String(mac[1], HEX) + ":" +
         String(mac[2], HEX) + ":" + String(mac[3], HEX) + ":" +
         String(mac[4], HEX) + ":" + String(mac[5], HEX);
}

void sendJson(JsonDocument doc) {
  if (idPlace != -1) {
    doc["id"] = idPlace;
  }
  doc["name"] = getMacAddress();  String json_str;
  serializeJson(doc, json_str);
  if (client.connected()) {
    webSocketClient.send(json_str);
    Serial.println("Request sent.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
  client.setInsecure(); // we live in a twilight world
  Serial.println("Waiting for connection.");
  while (!client.connect(host, 443)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Client connected.");
  webSocketClient.setCACert(ssl_cert);
  bool connected = webSocketClient.connect("wss://parking-rio.rezel.net/api");
  Serial.println("Waiting for handshake.");
  if (connected) {
    Serial.println("Connected");
  } else {
    Serial.println("Connection failed.");
  }

  webSocketClient.onMessage([&](WebsocketsMessage message) {
    if (client.connected()) {
      String data = message.data();

      if (data.length() > 0) {
        Serial.print("Received data: ");
        Serial.println(data);

        // Deserialize json
        JsonDocument receivedObj;
        deserializeJson(receivedObj, data);

        // Here answer to message
        const char *request = receivedObj["request"];
        const char *response = receivedObj["response"];
        JsonDocument toSendObj;
        if (strcmp(request, "name") == 0) {
          toSendObj["response"] = "name";
          if (idPlace == -1) {
            JsonDocument idRequestObj;
            idRequestObj["request"] = "getId";
            sendJson(idRequestObj);
          }
          sendJson(toSendObj);
        } else if (strcmp(response, "getId") == 0) {
          idPlace = receivedObj["id"];
        } else if (strcmp(request, "setId") == 0) {
          idPlace = receivedObj["id"];
        } else if (strcmp(request, "state") == 0) {
          toSendObj["response"] = "state";
          if (currentState != nullptr) {
            if (currentState == "busy" || currentState == "free" ||
                currentState == "reserved") {
              toSendObj["state"] = currentState;
            } else {
              toSendObj["state"] = "error";
            }
          }
        } else if (strcmp(request, "setState") == 0) {
          currentState = "reserved";
          previousState = currentState;
          toSendObj["response"] = "setState";
          toSendObj["state"] = currentState;
          sendJson(toSendObj);
        } else {
          toSendObj["response"] = "error";
          sendJson(toSendObj);
        }
      }
    }
  });
}

void switchLed(const String &color) {
  if (color == "red") {
    analogWrite(ledGreenPin, LOW);
    analogWrite(ledRedPin, HIGH);
  } else if (color == "green") {
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, HIGH);
  } else if (color == "orange") {
    digitalWrite(ledRedPin, HIGH);
    digitalWrite(ledGreenPin, HIGH);
  }
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * (soundSpeed/(2*pow(10,4)));
  String disp = String(distance);

  Serial.print("Distance: ");
  Serial.print(disp);
  Serial.println(" cm");
  
  if (distance <= 50) {
    currentState = "busy";
  } else {
      if (previousState != "reserved") {
        currentState = "free";
    } else {
      currentState = "reserved";
    }
  }
  
  if (currentState != previousState) {
    if (currentState == "busy") {
      switchLed("red");
    } else if (currentState == "reserved") {
      switchLed("orange");
    } else {
      switchLed("green");
    }
    
    JsonDocument toSendObj;
    toSendObj["request"] = "info";
    toSendObj["state"] = currentState;
    previousState = currentState;
    sendJson(toSendObj);
  }

  // Reset the timer after sending the WebSocket request
  lastTime = millis();

  // Add any additional logic or handling of WebSocket messages if needed
  webSocketClient.poll();
  delay(1000);
}
