#include <math.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "secret.h"

const String serverName = "https://parking-rio.rezel.net";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

const long soundSpeed = 343.2;
long duration, distance;

#define echoPin 2               // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN
#define trigPin 4               // CHANGE PIN NUMBER HERE IF YOU WANT TO USE A DIFFERENT PIN
const int ledRedPin = 23;
const int ledGreenPin = 19;

WiFiClientSecure client;

void setup() {
  Serial.begin(115200);
  pinMode(echoPin, INPUT);
  pinMode(trigPin, INPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void sendHTTPRequest(const String& endpoint) {
  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    client.setInsecure();
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    const String fullURL = serverName + "/api/" + endpoint;

    Serial.println("HTTP URL: " + fullURL);
    http.begin(client, fullURL);

    // Specify content-type header
    http.addHeader("Content-Type", "application/json");
    // Data to send with HTTP POST
    String httpRequestData = "";
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      // Get the response payload
      String responsePayload = http.getString();
      Serial.println("Response Data: " + responsePayload);
    } else {
      Serial.print("HTTP Request failed, error code: ");
      Serial.println(httpResponseCode);
    }

    // Free resources
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
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
    sendHTTPRequest("setOn");
    digitalWrite(ledGreenPin, LOW);
    digitalWrite(ledRedPin, HIGH);
  } else {
    sendHTTPRequest("setOff");
    digitalWrite(ledRedPin, LOW);
    digitalWrite(ledGreenPin, HIGH);
  }

  // Reset the timer after sending the HTTP request
  lastTime = millis();

  delay(1000);
}
