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
bool menuMode = true;
byte blinkIndex = 0;  // Index variable used when showing the LED pattern to the player
unsigned long blinkModeMillis = 0;
bool gameOver = false;    // Game goes to this mode if player presses wrong button or timeout triggers
bool gameRunning = true;  // In this mode a game "round" is running
unsigned int rounds = 0;  // Count how many games the player has played so far successfully

// Timing settings
unsigned int blinkDuration = 100;        // How long should the LEDs blink when showing the pattern?
unsigned int blinkDurationDecrease = 2;  // How much blink duration will decrease on every round
unsigned int minBlinkDuration = 50;      // Minimum amount for blink duration time, it cannot go under this
unsigned int blinkWait = 500;            // How long delay should there be before next LED in the sequence?
unsigned int blinkWaitDecrease = 30;     // Amount that the blink wait time decreases on every round
unsigned int minBlinkWait = 50;          // Minimum amount for blink wait, it cannot go under this
const int timeout = 10000;               // when to stop the game if no buttons are pressed
unsigned long previousTime = 0;          // store the previous timer, used to reset the timeout timer (don't touch this)


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

  if (menuMode) menu();

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

  // Decrease the blinking time and blink wait time to make the game more diffucult
  // Gredually reduce waitDecrease to reduce difficulty
  if (rounds % 5 == 0 && blinkWaitDecrease > 5) {
    blinkWaitDecrease -= 5;
  }

  if (blinkDuration >= minBlinkDuration) {
    blinkDuration -= blinkDurationDecrease;
  }
  if (blinkWait >= minBlinkWait) {
    blinkWait -= blinkWaitDecrease;
  }

  // Wait before showing the next sequence
  previousTime = millis();
  while (millis() - previousTime <= 2000) handleBlinking();
}

void menu() {
  leds[0] = CRGB::Green;
  leds[1] = CRGB::Yellow;
  leds[2] = CRGB::Red;
  FastLED.show();
  while (menuMode) {
    for (byte i = 0; i < 3; i++) buttons[i].loop();  // required by the ezButton library
    // Check if correct button is pressed

    for (byte i = 0; i < 3; i++) {
      if (buttons[i].isPressed()) {
        turnAllOff();
        setDifficulty(i);
        menuMode = false;
        while (buttons[i].isPressed()) buttons[i].loop();
        delay(1000);
        blinkModeMillis = millis();
        previousTime = millis();
        break;
      }
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
  Serial.print("blinking ");
  Serial.print(led);
  Serial.println("");
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
  randomSeed(analogRead(0));
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

void setDifficulty(byte difficulty) {
  Serial.print("Difficulty: ");
  switch (difficulty) {
    case 0:
    Serial.println("easy");
    blinkWaitDecrease = 10;
    break;
    case 1:
    Serial.println("medium");
    blinkWaitDecrease = 30;
    break;
    case 2:
    Serial.println("hard");
    blinkWaitDecrease = 60;
    break;
  }
}