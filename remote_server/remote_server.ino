#include <esp_now.h>
#include <WiFi.h>
uint8_t broadcastAddress[] = { 0x14, 0x2B, 0x2F, 0xAF, 0x0D, 0x18 };

typedef struct struct_message {
  // char a[32]; different data structure
  // int b;
  // float c;
  bool on;        // r
  bool playing;   // b
  bool unlocked;  // y
  float rate;     // g
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo1;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

const int redButtonPin = 19;  // Red Button Pin
const int redLedPin = 33;     // Red LED Pin
bool redButtonState = HIGH;   // Red Button State

const int blkButtonPin = 26;  // Black Button Pin
const int blkLedPin = 27;     // Black LED Pin
bool blkButtonState = HIGH;   // Black Button State

const int blueButtonPin = 25;  // Blue Button Pin
const int blueLedPin = 12;     // Blue LED Pin
bool blueButtonState = HIGH;   // Blue Button State

const int switchLedPin = 13;  // Switch LED Pin

unsigned long lastDebounceTimeRed = 0;
unsigned long lastDebounceTimeBlue = 0;
unsigned long lastDebounceTimeBlack = 0;
const unsigned long debounceDelay = 30;

const int potPin = 37;  // Potentiometer input pin

unsigned long lastPotReadTime = 0;
const unsigned long potReadInterval = 100;  // Read every 100ms

float lastRate = 0.0;              // Track last sent rate
const float rateThreshold = 0.01;  // Minimum change threshold to send update

void setup() {
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo1.peer_addr, broadcastAddress, 6);
  peerInfo1.channel = 0;
  peerInfo1.encrypt = false;
  if (esp_now_add_peer(&peerInfo1) != ESP_OK) {
    Serial.println("Failed to add peer 1");
    return;
  }

  pinMode(switchLedPin, OUTPUT);
  digitalWrite(switchLedPin, HIGH);  // Turn on the switch LED and stay on for the duration of the program

  pinMode(redButtonPin, INPUT_PULLUP);
  pinMode(blkButtonPin, INPUT_PULLUP);
  pinMode(blueButtonPin, INPUT_PULLUP);

  pinMode(redLedPin, OUTPUT);
  pinMode(blkLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);

  digitalWrite(redLedPin, HIGH);
  digitalWrite(blkLedPin, LOW);
  digitalWrite(blueLedPin, HIGH);

  // Setup for potentiometer
  pinMode(potPin, INPUT);


  myData.on = true;
  myData.playing = true;

  // Send updaet
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  Serial.begin(9600);
}

void loop() {
  handleBlueButton();
  handleBlackButton();
  handleRedButton();
  
  
  handlePotentiometer();
}

void handleRedButton() {
  bool reading = digitalRead(redButtonPin);

  if ((millis() - lastDebounceTimeRed) > debounceDelay) {
    if (reading != redButtonState) {
      redButtonState = reading;
      if (redButtonState == LOW) {
        digitalWrite(redLedPin, !digitalRead(redLedPin));
        myData.on = !myData.on;

        // Send update
        esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      }
    }
    lastDebounceTimeRed = millis();
  }
}

void handleBlackButton() {
  bool reading = digitalRead(blkButtonPin);

  if ((millis() - lastDebounceTimeBlack) > debounceDelay) {
    if (reading != blkButtonState) {
      blkButtonState = reading;
      if (blkButtonState == LOW) {
        digitalWrite(blkLedPin, !digitalRead(blkLedPin));
        myData.unlocked = !myData.unlocked;

        // Send update
        esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      }
    }
    lastDebounceTimeBlack = millis();
  }
}

void handleBlueButton() {
  bool reading = digitalRead(blueButtonPin);

  if ((millis() - lastDebounceTimeBlue) > debounceDelay) {
    if (reading != blueButtonState) {
      blueButtonState = reading;
      if (blueButtonState == LOW) {
        digitalWrite(blueLedPin, !digitalRead(blueLedPin));
        myData.playing = !myData.playing;

        // Send update
        esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      }
    }
    lastDebounceTimeBlue = millis();
  }
}

void handlePotentiometer() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastPotReadTime >= potReadInterval) {
    int potValue = analogRead(potPin);
    // Reverse Rate output
    float rate = 1 - (potValue / 4095.0);

    // Check if rate has changed enough to warrant sending an update
    if (abs(rate - lastRate) >= rateThreshold) {
      myData.rate = rate;
      lastRate = rate;

      // Send update via ESP-NOW
      esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

      // Debug output
      Serial.print("Potentiometer Value: ");
      Serial.print(potValue);
      Serial.print(" Rate: ");
      Serial.println(rate, 3);
    }

    lastPotReadTime = currentMillis;
  }
}