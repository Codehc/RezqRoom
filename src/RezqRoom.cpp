// Try this: https://gist.github.com/adi-g15/de41e96079a5b63045e86dc7c8c5c87e 
// to connect to ESP8266 when you actually get it idk

#include <Arduino.h>
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>

#include "RezqRoom.h"
#include "Keys.h"


const String ROOM = "BB8D7706-CB83-4339-918F-0E371D0AA36B";

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(115200);

  Serial.println("Configuring pins");

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Done configuring pins");
    
  printSeperationLine();

  Serial.println("Configuring LEDs");

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

  Serial.println("Done configuring LEDs");

  printSeperationLine();

  wifiReconnect();

  firebaseReconnect();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Lost connection to ");
    Serial.println(SSID_);
    wifiReconnect();
  }

  // Query Firebase and update local config
  configureFromDB(&room);

  for (int f = 0; f < 5; f++) {
    RoomFeatures feature = room.features[f];
    if (feature == LEDs) {
      for (int l = 0; l < NUM_LEDS - 1; l++) {
        if (room.ledStatus == OFF) {
          leds[l] = CRGB(0, 0, 0);
        } else if (room.ledStatus == WHITE) {
          leds[l] = CRGB(255, 255, 255);
        }
      }

      FastLED.show();
    }
  }

  delay(500); // 0.5s loop times (ideal. Assumes not blocked by trying to connect to wifi)
}

void wifiReconnect() {
  Serial.print("Connecting to ");
  Serial.print(SSID_);
  Serial.print(" ");

  WiFi.mode(WIFI_STA);

  WiFi.begin(SSID_, PSWRD); 

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    yield(); // Allow ESP8266 background operations
    delay(1000);
    i++;
    Serial.print(".");
  } Serial.print("\nSuccessfully connected to ");
  Serial.print(SSID_);
  Serial.print(" with IP ");
  Serial.print(WiFi.localIP());
  Serial.println(" (took ");
  Serial.print(i);
  Serial.println(" seconds)");

  printSeperationLine();
}

void firebaseReconnect() {
  Serial.println("Initializing Firebase connection");
  FirebaseAuth auth;

  // Define the FirebaseConfig data for config data
  FirebaseConfig config;

  // Assign the project host and api key (required)
  config.host = FIREBASE_HOST;

  config.api_key = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);

  Serial.println("Done initializing Firebase connection");

  printSeperationLine();
}

void configureFromDB(RoomConfig * config) {
  FirebaseData firebaseData;

  for (int i = 0; i < 5; i++) {
    config->features[i] = NO_FEATURE;
  }
  if (Firebase.getInt(firebaseData, ROOM + "/features") && firebaseData.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
    config->featuresBitmask = firebaseData.to<int>();

    // Check what features the featuresBitmask (bit representation of all the 5 possible features) stores
    // Compare it to a bitmask of all possible features and add them to the feature list of they exist
    int indexWritten = 0;
    if (config->featuresBitmask & LEDs) {
      config->features[indexWritten] = LEDs;
      indexWritten++;
    }
  }

  if (Firebase.getInt(firebaseData, ROOM + "/ledStatus") && firebaseData.dataTypeEnum() == fb_esp_rtdb_data_type_integer) {
    config->ledStatus = static_cast<LEDStatus>(firebaseData.to<int>());
  } else {
    config->ledStatus = OFF;
  }
}

void printSeperationLine() {
  Serial.println("------------------------------");
}
