#include <FastLED.h>

// RGB LED strip setup
const byte ledPin = 3;
const byte numLeds = 3;
CRGB leds[numLeds];

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, ledPin>(leds, numLeds);
}

void loop() {
  blink(0, 200, CRGB::Red);
  delay(1000);
  blink(1, 200, CRGB::Green);
  delay(1000);
  blink(2, 200, CRGB::Blue);
  delay(1000);
}

void blink(byte led, byte blinkTime, CRGB color) {
  leds[led] = color;
  FastLED.show();
  delay(blinkTime);
  leds[led] = CRGB::Black;
  FastLED.show();
}