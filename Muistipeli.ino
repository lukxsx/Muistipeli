#include <FastLED.h>
#include <ezButton.h>

#define SEQ_MAX_LEN 50
#define LED_PIN 3
#define NUM_LEDS 3

// RGB LED strip setup
CRGB leds[NUM_LEDS];
unsigned long lastBlinkTimes[] = { 0, 0, 0 };
int blinkTimes[] = { 0, 0, 0 };

// Buttons
const ezButton buttons[] = { 6, 7, 8 };

// Storing the pattern sequences here
byte sequence[SEQ_MAX_LEN] = { 0 };
byte seqIndex = 0;
byte seqLength = 1;  // Start with seqence with length of 1

// Game state variables
bool blinkMode = true;  // Blinking mode is the mode where the LEDs are blinking the sequence
byte blinkIndex = 0;
unsigned long blinkModeMillis = 0;
bool gameOver = false;

// Timing settings
unsigned int blinkDuration = 100;  // How long should the LEDs blink when showing the pattern?
unsigned int blinkWait = 500;      // How long delay should there be before next LED in the sequence?
const int timeout = 10000;         // when to stop the game if no buttons are pressed
unsigned long previousTime = 0;    // store the previous timer, used to reset the timeout timer (don't touch this)


void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  // Set button debouncing times
  for (byte i = 0; i < 3; i++) {
    buttons[i].setDebounceTime(50);
  }

  // Make sure all LEDs are off at the start
  turnAllOff();
}

void loop() {

  generatePattern(3);
  Serial.print("seq: ");
  for (byte i = 0; i < 3; i++) {
    Serial.print(sequence[i]);
    Serial.print(" ");
  }
  Serial.println("");
  while (!gameOver) {

    // Execute function that processes the LED and button states
    // (required to make them work in non-blocking way)
    handleBlinking();
    for (byte i = 0; i < 3; i++) buttons[i].loop();  // required by the ezButton library

    // In this mode, the sequence is blinked with the LEDs
    if (blinkMode) {
      if (blinkIndex >= seqLength) {  // Sequence finished
        blinkMode = false;            // Back to normal mode
        blinkModeMillis = 0;
        previousTime = 0;  // Set timeout
        blinkIndex = 0;
        seqIndex = 0;
        continue;
      }

      unsigned long currentMillis = millis();

      if (currentMillis - blinkModeMillis >= blinkWait) {
        blinkModeMillis = currentMillis;
        blink(sequence[blinkIndex], blinkDuration, CRGB::Yellow);
        blinkIndex++;
      }
    }

    // Check for timeout
    if (millis() - previousTime >= timeout) {
      Serial.println("timeout");
      gameOver = true;
    }

    // Check if pattern is completed
    if (seqIndex >= seqLength) {
      gameOver = true;
    }

    // Check if any button is pressed
    for (byte i = 0; i < 3; i++) {
      if (buttons[i].isPressed()) {
        Serial.print("Button ");
        Serial.print(i);
        Serial.println(" pressed");

        // Blink the LED in red color if pressed during the blink mode
        if (blinkMode) {
          blink(i, 100, CRGB::Red);
          break;
        }


        if (i == sequence[seqIndex]) {
          seqIndex++;
          blink(i, 100, CRGB::Green);
        } else {  // else the game is over
          Serial.print("i=");
          Serial.print(i);
          Serial.print(" want=");
          Serial.println(sequence[seqIndex]);
          blink(i, 100, CRGB::Red);
          gameOver = true;
        }
      }
    }
  }


  Serial.println("game over");
  //delay(1000);
  //turnAllOff();
  for (;;) handleBlinking();
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
  for (byte i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void generatePattern(byte length) {
  for (byte i = 0; i < length; i++) {
    sequence[i] = random(3);
    if (i > 0) {
      while (sequence[i] == sequence[i - 1]) {  // make sure there are no two same LEDs in a row
        sequence[i] = random(3);
      }
    }
  }
  seqLength = length;
}