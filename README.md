Arduino code for the cassette, remote control, and Simon input stations. 
Javascript code for the web-based prototype of the cassette's animations and states.

## Cassette
- Initializes 4 RGB LED strips with the `Fastled` library
- Receives data from the remote control over `esp_now` wifi server
- Object oriented approach for cassette, spools, and infinity loop pixels
- Randomly scheduled inputs for each cassette leg, filling up cassette in an average of 45min

## Remote
- Hardcoded to send data to the MAC address of the cassette's ESP32
- Controls on/off, play/pause, unlock state, cassette fill %
- Only transmits data when a value has changed

## Simon
Work in progress.

We have both the Simon game working and the input station leg LED animation working, but combining them is tricky. 
The Simon code relies heavily on `delay`, which interrupts the timing of the LED animations. 
Using a timer library is probably the best approach but still hard because we need many timed operations in sequence. Or a library that simulates multithreading on ESP32.

## Input Station Leg
This folder has the input station leg LED animation code working in isolation, just for reference.

## Simulation
p5.js prototype of the cassette animations and states.

The simulation represents each LED pixel as a small circle. The ratio of the cassette's dimensions should match the physical build, but the number of LED pixels is probably inaccurate.

Includes some web serial integration that is no longer maintained but could be reused to interface with the remote controller.
