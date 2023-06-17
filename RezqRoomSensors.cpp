#include <Adafruit_NeoPixel.h>

/* Define LED constants */
// The pin the LED Strip is plugged in to
const unsigned int LED_PIN = 6;
// # of pixels on the LED Strip
const unsigned int PIXELS = 60;

// Ticks per second for LED animations
const int TPS = 100;

/* Define sonar constants */
// Sonar Pins
const unsigned int SONAR_ONE_TRIG_PIN = 3;
const unsigned int SONAR_ONE_ECHO_PIN = 2;
const unsigned int SONAR_TWO_TRIG_PIN = 5;
const unsigned int SONAR_TWO_ECHO_PIN = 4;

const unsigned int BEFORE_LED = 0;
const unsigned int AFTER_LED = 1;

// Detecting object threshold
const int detectedThreshold = 110;
const float secondsToEndScan = 5;

// Debounce after walk in/out detection
const int ticksToDebounce = 0;

// Room state
unsigned int numPeople = 0;

/* States for state machine */
enum action
{
    TURNING_ON = 0,
    ON = 1,
    TURNING_OFF = 2,
    OFF = 3,
};

enum sonar
{
    BEFORE_DOOR = 0,
    AFTER_DOOR = 1,
};

enum sonarState
{
    NONE_DETECTED = 0,
    BEFORE_DETECTED = 1,
    AFTER_DETECTED = 2,
    BOTH_DETECTED = 3,
};

typedef struct
{
    int red;
    int green;
    int blue;
} RGBColor;

typedef struct
{
    RGBColor WHITE;
} colors;

const colors color = {
    .WHITE = (RGBColor){255, 255, 255},
};

int turnOn[12] = {5, 10, 20, 30, 50, 70, 110, 150, 200, 225, 250, 255};
int turnOff[12] = {255, 250, 225, 200, 150, 110, 70, 50, 30, 20, 10, 5};

/* Calculate MS per tick for animations */
const unsigned int MS_PER_TICK = 1000 / TPS;

/* Initiate variables */
Adafruit_NeoPixel pixels(PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Current state machine state
enum action currentState;

// Stores the last time a tick was done
unsigned long lastTick = 0;

// Debounce timer
int debounce = 0;

// Used for animations that require brightness
unsigned int currentAnimationTick = 0;

// Used to detect walking in/out
sonarState sonarHistory[4] = {NONE_DETECTED, NONE_DETECTED, NONE_DETECTED, NONE_DETECTED};
sonarState detectionHistory[2] = {NONE_DETECTED, NONE_DETECTED};

sonarState walkInPattern[4] = {BEFORE_DETECTED, NONE_DETECTED, AFTER_DETECTED, NONE_DETECTED};
sonarState altWalkInPattern[4] = {NONE_DETECTED, BEFORE_DETECTED, AFTER_DETECTED, NONE_DETECTED};

sonarState walkOutPattern[4] = {AFTER_DETECTED, NONE_DETECTED, BEFORE_DETECTED, NONE_DETECTED};
sonarState altWalkOutPattern[4] = {NONE_DETECTED, AFTER_DETECTED, BEFORE_DETECTED, NONE_DETECTED};

sonarState currentSonarState = NONE_DETECTED;

void setup()
{
    // Sets up sonar pins
    pinMode(SONAR_ONE_TRIG_PIN, OUTPUT);
    pinMode(SONAR_ONE_ECHO_PIN, INPUT);
    pinMode(SONAR_TWO_TRIG_PIN, OUTPUT);
    pinMode(SONAR_TWO_ECHO_PIN, INPUT);

    pinMode(BEFORE_LED, OUTPUT);
    pinMode(AFTER_LED, OUTPUT);

    // Resets LED pixels
    pixels.begin();

    pixels.clear();

    pixels.setBrightness(0);
    for (int i = 0; i < PIXELS; i++)
    {
        pixels.setPixelColor(i, pixels.Color(255, 255, 255));
    }

    // Initiates state machine
    currentState = OFF;

    Serial.begin(9600);
}

void loop()
{

    // pixels.clear();
    // pixels.setBrightness(10);
    // pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    // pixels.show();

    unsigned long currentTime = millis();

    // If enough time has passed for a new tick to happen, run the tick
    if (currentTime - lastTick >= MS_PER_TICK)
    {
        runPeriodic();
        lastTick = currentTime;
    }
}

void runPeriodic()
{
    enabledPeriodic();
    switch (currentState)
    {
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

void enabledPeriodic()
{
    if (debounce > 0)
    {
        debounce--;
    }
    else
    {
        // No item has triggered the detecting in/out process
        int beforeDoorDistance = querySonar(BEFORE_DOOR);
        int afterDoorDistance = querySonar(AFTER_DOOR);

        bool beforeDoorInRange = isInRange(beforeDoorDistance);
        bool afterDoorInRange = isInRange(afterDoorDistance);

        if (beforeDoorInRange && afterDoorInRange)
        {
            currentSonarState = BOTH_DETECTED;
        }
        else if (beforeDoorInRange)
        {
            currentSonarState = BEFORE_DETECTED;
        }
        else if (afterDoorInRange)
        {
            currentSonarState = AFTER_DETECTED;
        }
        else
        {
            currentSonarState = NONE_DETECTED;
        }

        addToDetectionHistory(currentSonarState);

        addToSonarHistory(getDetected());

        if (matchSonarHistoryPattern(walkInPattern) || matchSonarHistoryPattern(altWalkInPattern))
        {
            walkIn();
            debounce = ticksToDebounce;
            resetSonarHistory();
        }
        else if (matchSonarHistoryPattern(walkOutPattern) || matchSonarHistoryPattern(altWalkOutPattern))
        {
            walkOut();
            debounce = ticksToDebounce;
            resetSonarHistory();
        }
    }
}

void onPeriodic()
{
}

void offPeriodic()
{
}

void turningOnPeriodic()
{
    pixels.setBrightness(turnOn[currentAnimationTick]);
    pixels.show();

    currentAnimationTick++;

    if (currentAnimationTick > sizeof(turnOn) / sizeof(int))
    {
        currentState = ON;
    }
}

void turningOffPeriodic()
{
    pixels.setBrightness(turnOff[currentAnimationTick]);
    pixels.show();

    currentAnimationTick++;

    if (currentAnimationTick > sizeof(turnOff) / sizeof(int))
    {
        currentState = OFF;
    }
}

void walkIn()
{
    if (numPeople == 0)
    {
        currentState = TURNING_ON;
    }

    numPeople++;
    Serial.print("In: ");
    Serial.println(numPeople);
}

void addToSonarHistory(sonarState state)
{
    if (sonarHistory[3] == state)
        return;

    sonarHistory[0] = sonarHistory[1];
    sonarHistory[1] = sonarHistory[2];
    sonarHistory[2] = sonarHistory[3];
    sonarHistory[3] = state;
}

void addToDetectionHistory(sonarState state)
{
    detectionHistory[0] = detectionHistory[1];
    detectionHistory[1] = state;
}

sonarState getDetected()
{
    if (detectionHistory[0] == detectionHistory[1])
    {
        return detectionHistory[0];
    }
    else
    {
        return sonarHistory[3];
    }
}

bool matchSonarHistoryPattern(sonarState match[4])
{
    return match[0] == sonarHistory[0] && match[1] == sonarHistory[1] && match[2] == sonarHistory[2] && match[3] == sonarHistory[3];
}

void resetSonarHistory()
{
    sonarHistory[0] = NONE_DETECTED;
    sonarHistory[1] = NONE_DETECTED;
    sonarHistory[2] = NONE_DETECTED;
    sonarHistory[3] = NONE_DETECTED;
}

void walkOut()
{
    numPeople--;
    Serial.print("Out: ");
    Serial.println(numPeople);

    if (numPeople == 0)
    {
        currentState = TURNING_OFF;
    }
}

int querySonar(enum sonar sensor)
{
    int trigPin;
    int echoPin;

    if (sensor == BEFORE_DOOR)
    {
        trigPin = SONAR_ONE_TRIG_PIN;
        echoPin = SONAR_ONE_ECHO_PIN;
    }
    else if (sensor == AFTER_DOOR)
    {
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

bool isInRange(int distance)
{
    return distance > 1 && distance <= 30;
}