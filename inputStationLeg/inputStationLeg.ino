#include <FastLED.h>

#define LED_PIN       27
#define LED_COUNT_LEG 40
#define NUM_LEGS      6

#define PIXELS_PER_INPUT  8

CRGB legLeds[LED_COUNT_LEG * NUM_LEGS];
uint32_t pixels[LED_COUNT_LEG];

// Incoming pixel colors;
uint32_t colors[] = {
    0x000000, // off
    0x6600ff, // final purple color
    0x5f03ff, 
    0xb700ff,
    0xff00bb,
    0xFFA9FF
};
const size_t NUM_COLORS = sizeof(colors) / sizeof(colors[0]); 

unsigned long interval, timeout;
unsigned long lastTick;
int queue = 0;

void setup() {
  Serial.begin(115200);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(legLeds, LED_COUNT_LEG * NUM_LEGS);
  FastLED.setBrightness(255);
}

void loop() {
  if (millis() >= timeout) {
    Serial.println("ADD");
    queue += PIXELS_PER_INPUT;
    scheduleInput();
  }

  if (millis() - lastTick >= 50) {
    if (queue > 0) {
      queue--;
      unshift(pixels, LED_COUNT_LEG, colors[1]); // push purple pixel
    } else {
      unshift(pixels, LED_COUNT_LEG, colors[0]); // push dark pixel
    }
    lastTick = millis();
  }

  for(int i = 0; i < NUM_LEGS; i++) {
    for(int j = 0; j < LED_COUNT_LEG; j++) {
      uint32_t newColor;
      if (i % 2 == 0) { // strips numbered top to bottom
        newColor = pixels[j];
      } else { // strips numbered bottom to top
        newColor = pixels[LED_COUNT_LEG - j - 1];
      }
      legLeds[i * LED_COUNT_LEG + j] = CRGB(newColor);
    }
  }

  FastLED.show();
}

void scheduleInput() {
  interval = random(2000, 5000);
  timeout = millis() + interval;
}

void unshift(uint32_t arr[], int size, uint32_t newValue) {
  for (int j = size - 1; j > 0; j--) {
      arr[j] = arr[j - 1];
  }
  arr[0] = newValue;
}