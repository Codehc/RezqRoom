// Try this: https://gist.github.com/adi-g15/de41e96079a5b63045e86dc7c8c5c87e 
// to connect to ESP8266 when you actually get it idk
#define LED_PIN     1
#define NUM_LEDS    20


enum RoomFeatures {
  NO_FEATURE=0b00000, LEDs=0b00001
};

enum LEDStatus {
  OFF=0,
  WHITE=1
};

struct RoomConfig {
  int featuresBitmask;
  RoomFeatures features[5];
  LEDStatus ledStatus;
} room;

void wifiReconnect();

void firebaseReconnect(); 

void configureFromDB(RoomConfig * config); 

void printSeperationLine(); 
