// Simon game code downloaded from https://goodarduinocode.com/projects/simon

#include "pitches.h"
#include <FastLED.h>

#define LED_PIN     27   // 数据引脚（建议使用 GPIO 26）
#define NUM_LEDS    4    // LED 的数量

#define LED_PIN_LEG       33
#define LED_COUNT_LEG     40
#define NUM_LEG_COLUMNS   6

#define PIXELS_PER_INPUT   8

CRGB leds[NUM_LEDS];     // 定义一个数组存储 LED 的颜色信息
uint32_t Purple = 0x6600ff;
uint32_t Black = 0x000000;

// leg LED animation
CRGB legLeds[LED_COUNT_LEG * NUM_LEG_COLUMNS];
uint32_t pixels[LED_COUNT_LEG];

unsigned long interval, timeout;
unsigned long lastTick;
int queue = 0;

/* Constants - define pin numbers for LEDs,
   buttons and speaker, and also the game tones: */
// const byte ledPins[] = {13,12,27,33};
const int buttonPins[] = {25,4,19,21};
#define SPEAKER_PIN 7

#define MAX_GAME_LENGTH 100

const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};

/* Global variables - store the game state */
byte gameSequence[MAX_GAME_LENGTH] = {0};
byte gameIndex = 0;

// by default, tasks run on Core 1
// make 1 other task that will run on Core 2
TaskHandle_t TaskLeds;

/**
   Set up the Arduino board and initialize Serial communication
*/
void setup() {
  Serial.begin(9600);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); 
  FastLED.addLeds<WS2812, LED_PIN_LEG, GRB>(legLeds, LED_COUNT_LEG * NUM_LEG_COLUMNS);
  FastLED.setBrightness(255);

  for ( byte i = 0; i < 4; i++) {
    // pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(SPEAKER_PIN, OUTPUT);
  // The following line primes the random number generator.
  // It assumes pin A0 is floating (disconnected):
  randomSeed(analogRead(26));

  xTaskCreatePinnedToCore(
      updateLegLeds,    /* Function to implement the task */
      "TaskLeds",       /* Name of the task */
      10000,            /* Stack size in words */
      NULL,             /* Task input parameter */
      0,                /* Priority of the task */
      &TaskLeds,        /* Task handle. */
      0);               /* Core where the task should run */
}

/**
   Lights the given LED and plays a suitable tone
*/
void lightLedAndPlayTone(byte ledIndex) {
  leds[ledIndex] = Purple;
  FastLED.show();  
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  leds[ledIndex] = Black;
  FastLED.show();  
  noTone(SPEAKER_PIN);
}

/**
   Plays the current sequence of notes that the user has to repeat
*/
void playSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed);
    delay(50);
  }
}

/**
    Waits until the user pressed one of the buttons,
    and returns the index of that button
*/
byte readButtons() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.println(" low");
        return i;
      }
    }
    delay(1);
  }
}

/**
  Play the game over sequence, and report the game score
*/
void gameOver() {
  Serial.print("Game over! your score: ");
  Serial.println(gameIndex - 1);
  gameIndex = 0;
  delay(200);

  // Play a Wah-Wah-Wah-Wah sound
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(5);
    }
  }
  noTone(SPEAKER_PIN);
  delay(500);
}

/**
   Get the user's input and compare it with the expected sequence.
*/
bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton);
    if (expectedButton != actualButton) {
      return false;
    }
  }

  return true;
}

/**
   Plays a hooray sound whenever the user finishes a level
*/
void playLevelUpSound() {
  tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);
}

/**
   The main game loop
*/
void loop() {
  // Add a random color to the end of the sequence
  gameSequence[gameIndex] = random(0, 4);
  gameIndex++;
  if (gameIndex >= MAX_GAME_LENGTH) {
    gameIndex = MAX_GAME_LENGTH - 1;
  }
  playSequence();
  if (!checkUserSequence()) {
    gameOver();
  }

  delay(300);

  if (gameIndex > 0) {
    playLevelUpSound();
    delay(300);
  }
}

void updateLegLeds(void* pvParameters) {
  while(true) {
    if (millis() >= timeout) {
      Serial.println("ADD LEG PIXELS");
      queue += PIXELS_PER_INPUT;
      scheduleInputLeg();
    }

    delay(50); // controls speed of animation

    if (queue > 0) {
      queue--;
      unshift(pixels, LED_COUNT_LEG, Purple); // push purple pixel
    } else {
      unshift(pixels, LED_COUNT_LEG, Black); // push dark pixel
    }

    for(int i = 0; i < NUM_LEG_COLUMNS; i++) {
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
}

void scheduleInputLeg() {
  interval = random(2000, 5000);
  timeout = millis() + interval;
}

void unshift(uint32_t arr[], int size, uint32_t newValue) {
  for (int j = size - 1; j > 0; j--) {
      arr[j] = arr[j - 1];
  }
  arr[0] = newValue;
}