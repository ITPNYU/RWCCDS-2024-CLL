#include <Arduino.h>
#include <FastLED.h>

extern uint32_t colors[];
extern const size_t NUM_COLORS;

class Pixel {
  public:
    uint8_t ringNumber;
    uint8_t colorIdx;

    Pixel(uint8_t ringNumber = 0) {
      this->ringNumber = ringNumber;
      this->colorIdx = 0;
    }

    void setColor(uint8_t idx) {
      this->colorIdx = idx;
    }

    void draw(CRGB (&led), int fade=0) {
      if (this->colorIdx < NUM_COLORS) {
        uint32_t color = colors[this->colorIdx];
        led = CRGB(color).fadeLightBy(fade);
      }
    }
};
