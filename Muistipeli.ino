#include <FastLED.h>
#include <ezButton.h>

#define SEQ_MAX_LEN 50
#define LED_PIN 3
#define NUM_LEDS 3

// RGB LED strip setup
CRGB leds[NUM_LEDS];
unsigned long lastBlinkTimes[] = { 0, 0, 0 };
int blinkTimes[] = { 0, 0, 0 };

const ezButton buttons[] = { 6, 7, 8 };  // Buttons


byte sequence[SEQ_MAX_LEN] = { 0 };  // Storing the pattern sequences here
byte seqIndex = 0;
byte seqLength = 1;  // Start with seqence with length of 1

// Game state variables
bool blinkMode = true;  // Blinking mode is the mode where the LEDs are blinking the sequence
byte blinkIndex = 0;    // Index variable used when showing the LED pattern to the player
unsigned long blinkModeMillis = 0;
bool gameOver = false;    // Game goes to this mode if player presses wrong button or timeout triggers
bool gameRunning = true;  // In this mode a game "round" is running
unsigned int rounds = 0;  // Count how many games the player has played so far successfully

// Timing settings
unsigned int blinkDuration = 100;  // How long should the LEDs blink when showing the pattern?
unsigned int blinkWait = 500;      // How long delay should there be before next LED in the sequence?
const int timeout = 10000;         // when to stop the game if no buttons are pressed
unsigned long previousTime = 0;    // store the previous timer, used to reset the timeout timer (don't touch this)


void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

  // Set button debouncing times
  for (byte i = 0; i < 3; i++) buttons[i].setDebounceTime(50);

  // Make sure all LEDs are off at the start
  turnAllOff();
  delay(2000);
}

void loop() {
  // Create a new randomized pattern based on current length
  generatePattern(seqLength);

  printSequence();  // Debug printing

  // We start the round with blinking mode on (show sequence to player)
  gameRunning = true;
  blinkMode = true;

  // In this mode the game listens to player's input
  while (gameRunning) {
    // Execute function that processes the LED and button states
    // (required to make them work in non-blocking way)
    handleBlinking();
    for (byte i = 0; i < 3; i++) buttons[i].loop();  // required by the ezButton library

    // If we are in the blinking phase, show the pattern to player with LEDs
    if (blinkMode) {
      if (blinkIndex >= seqLength) {  // Sequence finished
        blinkMode = false;            // Back to normal mode
        blinkModeMillis = 0;
        previousTime = millis();  // Set timeout
        blinkIndex = 0;
        seqIndex = 0;
        continue;
      }

      // Wait a bit before showing the next LED in the sequence
      unsigned long currentMillis = millis();
      if (currentMillis - blinkModeMillis >= blinkWait) {
        blinkModeMillis = currentMillis;

        // Blink the LED and increase the blinking sequence index
        blink(sequence[blinkIndex], blinkDuration, CRGB::Yellow);
        blinkIndex++;
      }
    }

    // Check for timeout
    if (millis() - previousTime >= timeout) {
      Serial.println("timeout");
      gameOver = true;
      gameRunning = false;
    }

    // Check if pattern is completed
    if (seqIndex >= seqLength) gameRunning = false;

    // Check if any button is pressed
    for (byte i = 0; i < 3; i++) {
      if (buttons[i].isPressed()) {

        // Blink the LED in red color if pressed during the blink mode
        if (blinkMode) {
          blink(i, 100, CRGB::Red);
          break;
        }

        // Check if correct button is pressed
        if (i == sequence[seqIndex]) {
          // Yes, increase the sequence index and blink green LED
          seqIndex++;
          blink(i, 100, CRGB::Green);
        } else {
          // No. Blink red LED and end the game
          blink(i, 100, CRGB::Red);
          gameOver = true;
          gameRunning = false;
        }
      }
    }
  }

  // Loop endlessly
  if (gameOver) {
    Serial.println("game over");
    for (;;) handleBlinking();
  }

  // Increase the sequence length on every 2 rounds
  if (rounds % 2 == 0) seqLength++;
  rounds++;
  //seqLength++;
  seqIndex = 0;

  // TODO: Decrease the blinking time and blink wait time to make the game more diffucult

  // Wait before showing the next sequence
  previousTime = millis();
  while (millis() - previousTime <= 2000) handleBlinking();
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
  for (byte i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Black;
  FastLED.show();
}

// Generate randomized pattern
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

// Used for debugging
void printSequence() {
  Serial.print("seq: ");
  for (byte i = 0; i < seqLength; i++) {
    Serial.print(sequence[i]);
    Serial.print(" ");
  }
  Serial.println("");
}