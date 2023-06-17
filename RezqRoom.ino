#include <Arduino.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>

#include "ArrayUtils.h"
#include "Keys.h"

const char* PROFILE = "BB8D7706-CB83-4339-918F-0E371D0AA36B";

const int SENSOR_PIN = A0;

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing");

  pinMode(SENSOR_PIN, INPUT);

  Serial.println("Pins configured");

  Serial.print("Connecting to ");
  Serial.print(SSID_);
  Serial.print(" ");

  WiFi.mode(WIFI_STA);

  WiFi.begin(SSID_, PSWRD); 

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    i++;
    Serial.print(".");
  }
  Serial.print("\nSuccessfully connected to ");
  Serial.print(SSID_);
  Serial.print(" with IP ");
  Serial.print(WiFi.localIP());
  Serial.println(" (took ");
  Serial.print(i);
  Serial.println(" seconds)");

  firebaseReconnect();
}

void loop() {
  // Query Firebase and update local config
  FirebaseObject remoteConfigGet = Firebase.get(String("/profiles/") + PROFILE);
  if (Firebase.failed()) {
      Serial.println("Firebase get failed");
      Serial.println(Firebase.error());
  } else {
    config.enabled = remoteConfigGet.getBool("enabled");
  }
  
  delay(250);
}

  Serial.println("Initializing Firebase connection");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Serial.println("Firebase done initializing");
}

void printConfig(Config *config) {
  Serial.println("Config:");
  Serial.println(" - Using Purely Interval: " + String(config->usingPurelyInterval));
  Serial.println(" - Interval: " + String(config->interval));
  Serial.println(" - Watering Threshold: " + String(config->wateringThreshold));
  Serial.println(" - Watering Time: " + String(config->wateringTime));
}
