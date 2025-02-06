#include <sys/_stdint.h>
#include <Arduino.h>
#include <FastLED.h>
#include <math.h>

#define LED_COUNT             150       // Number of LEDs in infinity loop
#define PIXEL_TRAIL_COUNT     20        // Number of continuous LED pixels moving around the infinity loop
#define PIXEL_TRAIL_ALPHA     0.2       // Controls how quickly LED pixel trail fades in brightness. Used in exponential fn below

static uint32_t yellow = 0xffff00;  // LED pixel color used for infinity loop

/**
  This class represents the infinity loop on the cassette tape; the strip of LEDs going around the two spools.
  It responds to state changes dispatched by the remote control.
  The infinity loop is only on during "concert unlock" state, not while the spools are filling up in pre-concert.
*/

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

    int getBrightness(int pixelIndex, float alpha = PIXEL_TRAIL_ALPHA) {
      return (int)(255 * exp(-alpha * (PIXEL_TRAIL_COUNT - pixelIndex + 1)));
    }
};
