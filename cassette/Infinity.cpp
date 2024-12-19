#include <sys/_stdint.h>
#include <Arduino.h>
#include <FastLED.h>
#include <math.h>

#define LED_COUNT   150
#define PIXEL_TRAIL_COUNT 20
static uint32_t yellow = 0xffff00;

class Infinity {
  public:
    CRGB (&leds)[LED_COUNT];

    String state;
    int idx;

    Infinity(String state, CRGB (&leds)[LED_COUNT]): state(state), leds(leds) {
      this->idx = 0;
    }

    void switchState(String newState) {
      this->state = newState;
      if (newState == "off") {
        this->clear();
      }
    }

    void tick() {
      this->idx = (this->idx + 1) % LED_COUNT;
    }

    void draw() {
      this->clear();

      if (this->state == "unlock" || this->state == "concert") {
        for (int i = 0; i < PIXEL_TRAIL_COUNT; i++) {
          int index = (this->idx + i) % LED_COUNT; 
          int fade = 255 - this->getBrightness(i);
          leds[index] = CRGB(yellow).fadeLightBy(fade);
        }
      }
    }

    void clear() {
      for (int i = 0; i < LED_COUNT; i++) {
        leds[i] = CRGB(0x000000);
      }
    }

    int getBrightness(int pixelIndex, float alpha = 0.2) {
      return (int)(255 * exp(-alpha * (PIXEL_TRAIL_COUNT - pixelIndex + 1)));
    }
};
