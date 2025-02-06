#include <Arduino.h>
#include <FastLED.h>

#include "Spool.cpp"
#include "Infinity.cpp"

// These are also defined in cassette.ino but I don't know how to import them here, so redefining.
// These need to stay in sync across all files though

#define SPOOL_LED_COUNT     720     // Number of LEDs in one spool/spiral, including leg. 240 for spiral + 80 * 6 for leg
#define INFINITY_LED_COUNT  150     // Number of LEDs in infinity loop
#define OUTLINE_LED_COUNT   190     // Number of LEDs in border outline

// pixel colors array defined in cassette.ino
extern uint32_t colors[];

/**
  This class manages all state for the all of the LED strips. 
  It dispatches changes to sub-objects (Infinity and Spools), which directly control the LED pixels.
  It ticks each sub-object, draws them, and dispatches state changes.
*/

class Cassette {
  public:
    String state;
    Spool spoolLeft;
    Spool spoolRight;
    Infinity infinity;
    CRGB outlineLeds[OUTLINE_LED_COUNT];

    Cassette(
      CRGB (&spoolLeftLeds)[SPOOL_LED_COUNT], 
      CRGB (&spoolRightLeds)[SPOOL_LED_COUNT],
      CRGB (&infinityLeds)[INFINITY_LED_COUNT],
      CRGB (&outlineLeds)[OUTLINE_LED_COUNT]
    ) : 
      state("start"), 
      spoolLeft(state, spoolLeftLeds), 
      spoolRight(state, spoolRightLeds), 
      infinity(state, infinityLeds) {
        this->switchState("start");
        memcpy(this->outlineLeds, outlineLeds, sizeof(this->outlineLeds));
      }

    void switchState(String newState) {
      this->state = newState;
      if (newState == "off") {
        drawOutline(false);
      }

      spoolLeft.switchState(newState); 
      spoolRight.switchState(newState); 
      infinity.switchState(newState);
    }

    void tick() {
      spoolLeft.tick();
      spoolRight.tick();
      infinity.tick();
    }

    void draw() {
      if (this->state != "off") {
        Serial.println("state is not off");
        drawOutline(true);
      } else {
        drawOutline(false);
      }

      spoolLeft.draw();
      spoolRight.draw();
      infinity.draw();
    }

    void drawOutline(bool on) {
      for(int i = 0; i < OUTLINE_LED_COUNT; i++) {
        if (on) {
          this->outlineLeds[i] = CRGB::Purple;
        } else {
          this->outlineLeds[i] = CRGB::Black;
        }
      }
    }

    void setPercentFill(float fill) {
      spoolLeft.setPercentFill(fill);
      spoolRight.setPercentFill(fill);
    }
};
