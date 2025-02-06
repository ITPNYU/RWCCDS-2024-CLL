#include <sys/_stdint.h>
#include <Arduino.h>
#include <FastLED.h>
#include <math.h>
#include "Pixel.cpp"

#define PIXEL_COLOR_PER_INPUT   2       // Number of duplicates of each color pixel in leg LED inputs. Can be changed. Ex: 2 means white white, pink pink, purple purple.
#define SPOOL_ROTATE_DEGREES    5       // Number of degrees spool should rotate in one frame during concert phase. Bigger number = faster rotation.

#define LED_COUNT               240     // Number of LED pixels on one spool/spiral. Keep in sync with cassette.ino. Physically constant.
#define LEG_LED_COUNT           80      // Number of LED pixels in one COLUMN of cassette leg. Keep in sync with cassette.ino. Physically constant.
#define LEG_LED_COLUMNS         6       // Number of columns of LEDs on one cassette leg. Keep in sync with cassette.ino. Physically constant.


// all defined in cassette.ino
extern uint32_t colors[];
extern int rings[5][2];
extern const size_t NUM_COLORS;
extern const int INPUTS_PER_RING;

const int TOTAL_LEDS = LED_COUNT + LEG_LED_COUNT * LEG_LED_COLUMNS;

/**
  This class represents one spool/spiral on the cassette tape. 
  These are the LED strips laid out in a spiral shape.
  The spool LEDs are directly connected to the cassette's leg LEDs, 
  so from the code's perspective the leg is part of this Spool.

  Here there be dragons, you probably don't want to mess with this math.
*/

class Spool {
  public:
    Pixel pixels[LED_COUNT + LEG_LED_COUNT];
    uint8_t pixelColors[LED_COUNT + LEG_LED_COUNT];
    CRGB (&leds)[TOTAL_LEDS];


    // inline comment these
    String state;
    int pixelsCounter;
    int pixelsPerCircle;
    int maxPixels;
    int maxCircles;
    int queue;
    int i;
    bool rotating;
    int rotateAngle;

    Spool(String state, CRGB (&leds)[TOTAL_LEDS]): state(state), leds(leds) {
      this->pixelsPerCircle = INPUTS_PER_RING * PIXEL_COLOR_PER_INPUT * NUM_COLORS;
      this->pixelsCounter = 0;
      this->maxCircles = sizeof(rings) / sizeof(rings[0]);
      this->maxPixels = this->pixelsPerCircle * this->maxCircles;
      this->i = 0;
      this->rotating = false;
      this->rotateAngle = 0;

      for (int i = 0; i < LED_COUNT + LEG_LED_COUNT; i++) {
        bool hasRing = false;
        for(int ring = 0; ring < this->maxCircles; ring++) {
          if (i >= rings[ring][0] && i < rings[ring][1]) {
            pixels[i] = Pixel(ring+1);
            hasRing = true;
          }
        }
        if (!hasRing) {
          pixels[i] = Pixel();
        }
      }
    }

    void switchState(String newState) {
      this->state = newState;
      if (newState == "unlock" || newState == "concert") {
        this->pixelsCounter = this->maxCircles * this->pixelsPerCircle;
      } else if (newState == "off") {
        this->pixelsCounter = 0;
        this->off();
      } else {
        this->pixelsCounter = 0;
      }

      this->rotating = newState == "unlock" || newState == "concert";
    }

    void setPercentFill(float fill) {
      this->pixelsCounter = this->maxPixels * fill;
    } 

    void addPixels() {
      if (this->state == "start") {
        this->queue += NUM_COLORS * PIXEL_COLOR_PER_INPUT;
      }
    }

    void tick() {
      if (this->queue > 0) {
        this->queue--;
        uint8_t index = this->i / PIXEL_COLOR_PER_INPUT;
        if (index < NUM_COLORS) {
          unshift(pixelColors, LED_COUNT + LEG_LED_COUNT, index);
        }
        this->i = (this->i + 1) % (NUM_COLORS * PIXEL_COLOR_PER_INPUT);
      } else {
        unshift(pixelColors, LED_COUNT + LEG_LED_COUNT, 0);
      }

      if (pixelColors[LED_COUNT + LEG_LED_COUNT - 1] != 0x0 && this->pixelsCounter < this->maxPixels) {
        this->pixelsCounter++;
      }

      if(this->rotating) {
        this->rotate();
      }
    }

    void draw() {
      // swirling in pixels in spiral + first leg column
      for (int i = 0; i < LED_COUNT + LEG_LED_COUNT; i++) {
        pixels[i].setColor(pixelColors[i]);
        pixels[i].draw(leds[LED_COUNT + LEG_LED_COUNT - i - 1]); 
      }

      // moving pixels up leg columns
      for(int i = 1; i < LEG_LED_COLUMNS; i++) {
        for(int j = 0; j < LEG_LED_COUNT; j++) {
          uint8_t baseIndex = LEG_LED_COUNT - j - 1;
          uint8_t newColorIdx;
          if (i % 2 == 0) { // Even columns (bottom-to-top)
              newColorIdx = pixelColors[baseIndex];
          } else { // Odd columns (top-to-bottom)
              newColorIdx = pixelColors[j];
          }

          leds[LED_COUNT + i * LEG_LED_COUNT + j] = CRGB(colors[newColorIdx]);
        }
      }

      int filledRings = floor(this->pixelsCounter / this->pixelsPerCircle);
      int pixelsInCurrentRing = this->pixelsCounter % this->pixelsPerCircle;

      // spiral
      for (int i = 0; i < LED_COUNT; ++i) {
        Pixel &pixel = pixels[i];
        if (pixel.ringNumber <= filledRings && pixel.ringNumber > 0) {
          pixel.setColor(1);
          pixel.draw(leds[i]);
        } else if (pixel.ringNumber == filledRings + 1 && pixel.ringNumber > 0) {
          int fade = map(pixelsInCurrentRing, 0, this->pixelsPerCircle, 255, 0);
          if (fade != 255) {
            pixel.setColor(1);
            pixel.draw(leds[i], fade);
          }
        }
      }

      if (this->rotating && this->state == "unlock" || this->state == "concert") {
        for(int i = 0; i < this->maxCircles; i++) {
          int ringStart = rings[i][0];
          int ringEnd = rings[i][1];
          if (i == this->maxCircles-1) { // last ring is partial
            ringEnd *= 1.1; //1.55;
          }

          int ringSize = ringEnd - ringStart;

          int sliceCount = (int)(ringSize * 0.12);
          int sliceCenter = (int)((this->rotateAngle / 360.0) * ringSize) + ringStart;
          int startIndex = sliceCenter - sliceCount / 2;
          int endIndex = startIndex + sliceCount;

          if (startIndex < ringStart) {
            startIndex = ringEnd - (ringStart - startIndex);
          }
          if (endIndex > ringEnd) {
            endIndex = ringStart + (endIndex - ringEnd);
          }

          for (int j = ringStart; j < ringEnd; j++) {
            // Wrap the indices around the ring
            int wrappedJ = (j - ringStart) % ringSize + ringStart;

            if (startIndex <= endIndex) {
              if (wrappedJ >= startIndex && wrappedJ < endIndex) {
                leds[wrappedJ] = CRGB::Black;
              }
            } else {
              // Case: Slice wraps around the ring
              if (wrappedJ >= startIndex || wrappedJ < endIndex) {
                leds[wrappedJ] = CRGB::Black;
              }
            }
          }
        }
      }
    }

    void rotate() {
      this->rotateAngle += SPOOL_ROTATE_DEGREES;
      if (this->rotateAngle >= 360) {
        this->rotateAngle = 0; 
      }
    }

    void unshift(uint8_t arr[], int size, uint8_t newValue) {
      for (int j = size - 1; j > 0; j--) {
          arr[j] = arr[j - 1];
      }
      arr[0] = newValue;
    }

    void off() {
      for (int i = 0; i < TOTAL_LEDS; i++) {
        leds[i] = CRGB::Black;
      }
    }
};
