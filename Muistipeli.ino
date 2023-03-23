#include <FastLED.h>
#include <ezButton.h>

// RGB LED strip setup
const byte ledPin = 3;
const byte numLeds = 3;
CRGB leds[numLeds];
unsigned long lastBlinkTimes[] = { 0, 0, 0 };
int blinkTimes[] = { 0, 0, 0 };

// Buttons
const ezButton buttons[] = { 6, 7, 8 };

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, ledPin>(leds, numLeds);

  // Set button debouncing times
  for (byte i = 0; i < 3; i++) {
    buttons[i].setDebounceTime(50);
  }

  // Make sure all LEDs are off at the start
  turnAllOff();
}

void loop() {
  // Execute function that processes the LED and button states
  // (required to make them work in non-blocking way)
  handleBlinking();
  for (byte i = 0; i < 3; i++) buttons[i].loop();  // required by the ezButton library

  // Check if any button is pressed
  for (byte i = 0; i < 3; i++) {
    if (buttons[i].isPressed()) {
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" pressed");
      blink(i, 200, CRGB::Blue);
    }
  }
}

// Handles non-blocking LED blinking stuff
// Must be called all the time in order to work
void handleBlinking() {
  unsigned long currentTime = millis();
  for (byte i = 0; i < 3; i++) {
    if (currentTime - lastBlinkTimes[i] >= blinkTimes[i]) {
      lastBlinkTimes[i] = 0;
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
}

// Make one of the LEDs blink
void blink(byte led, unsigned int blinkDuration, CRGB color) {
  leds[led] = color;
  lastBlinkTimes[led] = millis();
  blinkTimes[led] = blinkDuration;
}

void turnAllOff() {
  for (byte i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}