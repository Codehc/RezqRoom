#include <Adafruit_NeoPixel.h>

/* Define LED constants */
// The pin the LED Strip is plugged in to
const unsigned int LED_PIN = 6;
// # of pixels on the LED Strip
const unsigned int PIXELS = 60;

// Ticks per second for LED animations
const int TPS = 1;

/* Define sonar constants */
// Sonar Pins
const unsigned int SONAR_ONE_TRIG_PIN = 3;
const unsigned int SONAR_ONE_ECHO_PIN = 2;
const unsigned int SONAR_TWO_TRIG_PIN = 5;
const unsigned int SONAR_TWO_ECHO_PIN = 4;

// Detecting object threshold
const int detectedThreshold = 110;
const float secondsToEndScan = 5;

// Room state
unsigned int numPeople = 0;

/* States for state machine */
enum action {
  TURNING_ON = 0,
  ON = 1,
  TURNING_OFF = 2,
  OFF = 3,
};

enum sonar {
  BEFORE_DOOR = 0,
  AFTER_DOOR = 1,
};

typedef struct {
  int red;
  int green;
  int blue;
} RGBColor;

typedef struct {
  RGBColor WHITE;
} colors;

const colors color = {
  .WHITE = (RGBColor) {255, 255, 255},
};

/* Calculate MS per tick for animations */
const unsigned int MS_PER_TICK = 1000 / TPS;

/* Initiate variables */
Adafruit_NeoPixel pixels(PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Current state machine state
enum action currentState;

// Stores the last time a tick was done
unsigned long lastTick = 0;

// Used for animations that require brightness
unsigned int currentBrightness = 0;

// Used to detect walking in/out
int heightDetected = 0;
enum sonar sensorDetected = BEFORE_DOOR;
unsigned long timeToEndScan = 0;

void setup() {
  // Sets up sonar pins
  pinMode(SONAR_ONE_TRIG_PIN, OUTPUT);
  pinMode(SONAR_ONE_ECHO_PIN, INPUT);
  pinMode(SONAR_TWO_TRIG_PIN, OUTPUT);
  pinMode(SONAR_TWO_ECHO_PIN, INPUT);
  
  // Resets LED pixels
  pixels.begin();
  pixels.clear();

  pixels.setBrightness(0);
  for (int i = 0; i < PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(255, 255, 255));
  }

  // Initiates state machine
  currentState = OFF;

  Serial.begin(9600);
}

void loop() {
  
  //pixels.clear();
  //pixels.setBrightness(10);
  //pixels.setPixelColor(0, pixels.Color(255, 255, 255));
  //pixels.show();

  unsigned long currentTime = millis();

  // If enough time has passed for a new tick to happen, run the tick
  if (currentTime - lastTick >= MS_PER_TICK) {
    runPeriodic();
    lastTick = currentTime;
  }
}

void runPeriodic() {
  enabledPeriodic();
  switch (currentState) {
    case TURNING_ON:
      turningOnPeriodic();
      break;
    case TURNING_OFF:
      turningOffPeriodic();
      break;
    case ON:
      onPeriodic();
      break;
    case OFF:
      offPeriodic();
      break;
  }
}

void enabledPeriodic() {
  unsigned long currentTime = millis();
  if (timeToEndScan < currentTime) {
    // No item has triggered the detecting in/out process
    int beforeDoorDistance = querySonar(BEFORE_DOOR);
    int afterDoorDistance = querySonar(AFTER_DOOR);

    if (beforeDoorDistance <= detectedThreshold) {
      heightDetected = beforeDoorDistance;
      sensorDetected = BEFORE_DOOR;
      timeToEndScan = currentTime + (secondsToEndScan * 1000);
    } else if (afterDoorDistance <= detectedThreshold) {
      heightDetected = afterDoorDistance;
      sensorDetected = AFTER_DOOR;
      timeToEndScan = currentTime + (secondsToEndScan * 1000);
    }
  } else {
    // Some item has triggered the detection of in/out process
    int distance;
    if (sensorDetected == BEFORE_DOOR) {
      distance = querySonar(AFTER_DOOR);
    } else {
      distance = querySonar(BEFORE_DOOR);
    }

    if (distance < heightDetected + 20 && distance > heightDetected - 20) {
      if (sensorDetected == BEFORE_DOOR) {
        walkIn();
      } else {
        walkOut();
      }
    }
  }
}

void onPeriodic() {

}

void offPeriodic() {
  
}

void turningOnPeriodic() {
  unsigned int newBrightness = currentBrightness += 10;

  if (newBrightness >= 255) {
    newBrightness = 255;
    currentState = ON;
  }

  pixels.setBrightness(newBrightness);
  pixels.show();

  currentBrightness = newBrightness;
}


void turningOffPeriodic() {
  // Can't be unsigned since it might jump into the negatives
  int newBrightness = currentBrightness -= 10;

  if (newBrightness <= 0) {
    newBrightness = 0;
    currentState = OFF;
  }

  pixels.setBrightness(newBrightness);
  pixels.show();

  currentBrightness = newBrightness;
}

int querySonar(enum sonar sensor) {
  int trigPin;
  int echoPin;

  if (sensor == BEFORE_DOOR) {
    trigPin = SONAR_ONE_TRIG_PIN;
    echoPin = SONAR_ONE_ECHO_PIN;
  } else if (sensor == AFTER_DOOR) {
    trigPin = SONAR_TWO_TRIG_PIN;
    echoPin = SONAR_TWO_ECHO_PIN;
  }

  // Clears the trigger
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Triggers sonar
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Measure time for ping to return
  unsigned long duration = pulseIn(echoPin, HIGH);

  // Does the math!!! crazy hard LOL!!!
  return duration * 0.034 / 2;
}

void walkIn() {
  if (numPeople == 0) {
    currentState = TURNING_ON;
  }

  numPeople++;
  Serial.println("Someone walked in. Num people: " + numPeople);
}

void walkOut() {
  numPeople--;
  Serial.println("Someone walked out. Num people: " + numPeople);

  if (numPeople == 0) {
    currentState = TURNING_OFF;
  }
}