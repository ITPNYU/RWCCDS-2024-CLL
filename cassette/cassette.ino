// #include <Adafruit_NeoPixel.h>
// #ifdef __AVR__
//  #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
// #endif
#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>
#include "Cassette.cpp"

#define LED_PIN_LEFT_SPOOL      32    // ESP32 pin for left spool/spiral LEDs on cassette
#define LED_PIN_RIGHT_SPOOL     15    // ESP32 pin for right spool/spiral LEDs on cassette
#define LED_PIN_INFINITY        27    // ESP32 pin for infinity loop LEDs around spirals on cassette
#define LED_PIN_OUTLINE         33    // ESP32 pin for LED outlined border on cassette

// these are redefined in other files because I'm bad at C++.
// !!! the values need to stay in sync across all files
#define SPOOL_LED_COUNT         240   // Number of LED pixels on one spool/spiral
#define INFINITY_LED_COUNT      150   // Number of LED pixels in the infinity loop
#define LEG_LED_COUNT           80    // Number of LED pixels in one COLUMN of cassette leg
#define LEG_LED_COLUMNS         6     // Number of columns of LEDs on one cassette leg
#define OUTLINE_LED_COUNT       190   // Number of LED pixels in cassette outline

#define LED_ANIMATION_UPDATE    50    // milliseconds between LED animation updates. Smaller number = faster animation

// LED strips
CRGB spoolLeftLeds[SPOOL_LED_COUNT + LEG_LED_COUNT * LEG_LED_COLUMNS];    // spool and leg LED strips are connected, one continuous line of LEDs
CRGB spoolRightLeds[SPOOL_LED_COUNT + LEG_LED_COUNT * LEG_LED_COLUMNS];
CRGB infinityLeds[INFINITY_LED_COUNT];
CRGB outlineLeds[OUTLINE_LED_COUNT];

// Cassette model
Cassette cassette(spoolLeftLeds, spoolRightLeds, infinityLeds, outlineLeds);

// LED pixel colors used in the cassette spools and legs
// The order of these colors within the array matters!
// Index 0 is saved for black/off
// Index 1 is the final purple color after the LEDs spiral in
// The rest are several colors up from the legs, spiraling into the cassette spools
uint32_t colors[] = {
    0x000000, // off
    0x6600ff, // final purple color
    0x5f03ff, // purple
    0xb700ff, // magenta
    0xff00bb, // hot pink
    0xFFA9FF, // light pink/white
};
const size_t NUM_COLORS = sizeof(colors) / sizeof(colors[0]); 

// Defines which LED pixels belong to which ring/layer of the cassette spools
// Rings fill up one by one, starting from Ring 0 which is the smallest, innermost ring
// These numbers need to be tweaked by visual trial and error to see where the ring boundaries fall best
int rings[5][2] = {
  {0, 36},
  {36, 81},
  {81, 133},
  {133,191},
  {191, SPOOL_LED_COUNT - 20},
};

// used for timing random input bursts up each cassette leg
unsigned long intervalLeft, intervalRight, timeoutLeft, timeoutRight;
unsigned long lastTick;

const int INPUT_INTERVAL_MIN = 10000; // millisecond minimum interval between random leg inputs
const int INPUT_INTERVAL_MAX = 20000; // millisecond maximum interval between random leg inputs

const int NUM_RINGS = 5; // physically constant LED layout, see above about rings/layers
const int PRECONCERT_DURATION_MINUTES = 45;
const int INPUT_INTERVAL_AVG_MS = (INPUT_INTERVAL_MAX + INPUT_INTERVAL_MIN) / 2000;

// FORMULA FOR INPUTS PER RING
// 5 rings * x inputs/ring * 15 seconds/input = 2700 seconds = 45 minutes
// 45min / 5 rings / 15 seconds = x inputs (36 in this example)
const int INPUTS_PER_RING = PRECONCERT_DURATION_MINUTES * 60 / INPUT_INTERVAL_AVG_MS / NUM_RINGS;

// Structure of data received from remote when any remote setting is updated
typedef struct struct_message {
  bool on;
  bool playing;
  bool unlocked;
  float rate;
} struct_message;

struct_message receivedData = {true, true, false, 0.0};
struct_message previousData = {true, true, false, 0.0};
bool messageReceived = false;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  previousData = receivedData;
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  messageReceived = true;

  // DEBUG REMOTE CONTROLLER DATA
  // Serial.print("ON: ");
  // Serial.println(receivedData.on);
  // Serial.println();
  // Serial.print("PLAYING: ");
  // Serial.println(receivedData.playing);
  // Serial.println();
  // Serial.print("UNLOCKED: ");
  // Serial.println(receivedData.unlocked);
  // Serial.println();
  // Serial.print("INPUT RATE: ");
  // Serial.println(receivedData.rate);
  // Serial.println();
}

void setup() {
  Serial.begin(115200);

  // Important! Initialize all LED strips to correct ESP32 pins and array lengths
  FastLED.addLeds<WS2812, LED_PIN_LEFT_SPOOL, GRB>(spoolLeftLeds, SPOOL_LED_COUNT + LEG_LED_COUNT * LEG_LED_COLUMNS);
  FastLED.addLeds<WS2812, LED_PIN_RIGHT_SPOOL, GRB>(spoolRightLeds, SPOOL_LED_COUNT + LEG_LED_COUNT * LEG_LED_COLUMNS);
  FastLED.addLeds<WS2812, LED_PIN_INFINITY, GRB>(infinityLeds, INFINITY_LED_COUNT);
  FastLED.addLeds<WS2812, LED_PIN_OUTLINE, GRB>(outlineLeds, OUTLINE_LED_COUNT);

  FastLED.setBrightness(128);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  for(int i = 0; i < OUTLINE_LED_COUNT; i++) {
    outlineLeds[i] = CRGB(colors[1]); // purple
  }

  if (cassette.state == "start") {
    scheduleInputLeft();
    scheduleInputRight();
  }
}

void loop() {
  if (messageReceived) { // any message from the remote controller?

    // handle remote control ON/OFF button states
    if (previousData.on != receivedData.on) {
      if (receivedData.on) { // off -> on
        Serial.println("State ON");
        cassette.switchState("start");
        scheduleInputLeft();
        scheduleInputRight();
      } else { // on -> off
        Serial.println("State OFF");
        cassette.switchState("off");
        FastLED.show();
      }
    }

    // handle remote control CONCERT UNLOCK state
    if (previousData.unlocked != receivedData.unlocked) {
      if(receivedData.unlocked) { // filling up -> unlocked
        Serial.println("State UNLOCK");
        cassette.switchState("unlock");
      } else { // unlock -> filling up
        Serial.println("State START");
        cassette.switchState("start");
      }
    }

    // handle remote control PLAY/PAUSE state
    if (previousData.playing != receivedData.playing) {
      if(receivedData.playing) { // paused -> playing
        Serial.println("State PLAY");
      } else { // playing -> paused
        Serial.println("State PAUSE");
      }
    }

    // handle remote control cassette spool fill percentage
    // hard control over percentage of the spool LEDs that are on
    if (previousData.rate != receivedData.rate) {
      cassette.setPercentFill(receivedData.rate);
    }

    messageReceived = false;
  }

  if (cassette.state == "off" || !receivedData.playing) {
    return; // do nothing
  }

  if (millis() >= timeoutLeft) {
    cassette.spoolLeft.addPixels();
    Serial.println("LEFT");
    scheduleInputLeft();
  }

  if (millis() >= timeoutRight) {
    cassette.spoolRight.addPixels();
    Serial.println("RIGHT");
    scheduleInputRight();
  }

  if (millis() - lastTick >= LED_ANIMATION_UPDATE) {
    cassette.tick(); // !!! important! updates our managed LED state
    lastTick = millis();
  }

  // !!! important! applies managed LED state to LED strips
  cassette.draw(); 

  // !!! important! makes LED updates actually visible
  FastLED.show();
}

void scheduleInputLeft() {
  intervalLeft = random(INPUT_INTERVAL_MIN, INPUT_INTERVAL_MAX);
  timeoutLeft = millis() + intervalLeft;
}

void scheduleInputRight() {
  intervalRight = random(INPUT_INTERVAL_MIN, INPUT_INTERVAL_MAX);
  timeoutRight = millis() + intervalRight;
}

// call this function from loop() to help visualize parts of the LED strips
void debugLegs() {
  for(int i = 0; i < 242; i++) {
    spoolLeftLeds[i] = CRGB::Green;
    spoolRightLeds[i] = CRGB::Green;
  }
  for(int i = 0; i < LEG_LED_COLUMNS; i++) {
    for(int j = 0; j < LEG_LED_COUNT; j++) {
      int color;
      if (i == 0) {
        color = CRGB::Blue;
      } else if (i == 1) {
        color = CRGB::Cyan;
      }
      else if (i == 2) {
        color = CRGB::Red;
      }
      else if (i == 3) {
        color = CRGB::Pink;
      }
      else if (i == 4) {
        color = CRGB::Green;
      }
      else if (i == 5) {
        color = CRGB::Red;
      }
      spoolLeftLeds[i * LEG_LED_COUNT + 242 + j] = color;
    }
  }
}
