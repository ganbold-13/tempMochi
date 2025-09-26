#include <Arduino.h>
#include <U8g2lib.h>

#include <Adafruit_NeoPixel.h>
#include "pitches.h"
#include "messages.h"
#include "images.h"
#include "audios.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const int songspeed = 1.5;

#define PIN 7
#define NUMPIXELS 2

#define U8X8_PIN_NONE -1
#define jsupPin 20
#define jsdownPin 2
#define jsrightPin 10
#define jsleftPin 4
#define jsswitchPin 3
#define reedPin 21
#define buttonAPin 5
#define buttonBPin 6

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

char modeState = 0;
int menuIndex = 0;
const int menuLength = 3;
const char* menuItems[] = {"Fun Fact or Dad Joke", "GAME", "MESSAGE"};

bool jsupVal = 0;
bool jsdownVal = 0;
bool jsrightVal = 0;
bool jsleftVal = 0;
bool jsswitchVal = 0;

bool buttonAVal = 0;
bool buttonBVal = 0;

bool ledVal = 0;
unsigned long prevtimer = 0;
int interval = 1000;
int count = 0;
bool reedVal = 0;

int noteSize;
int randForJ;
int startSCR;

int totalLines;
int scrollIndex = 0;

// Snake Game Constants (from previous Snake code)
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;
const int BLOCK_SIZE = 8;
const int MAX_SNAKE_LENGTH = 50;

int snakeX[MAX_SNAKE_LENGTH], snakeY[MAX_SNAKE_LENGTH];
int snakeLength = 5;
int foodX, foodY;
int directionX = 1, directionY = 0;
bool gameOver = false;
unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 200;

#define JOY_UP    jsupPin    // Up
#define JOY_DOWN  jsdownPin  // Down
#define JOY_LEFT  jsleftPin  // Left
#define JOY_RIGHT jsrightPin // Right

void runSnakeGame() {
  unsigned long currentTime = millis();

  if (currentTime - lastMoveTime > moveDelay) {
    // Read joystick input (using your pin definitions)
    if (digitalRead(JOY_UP) == LOW && directionY != 1) { // Up
      directionX = 0;
      directionY = -1;
    } else if (digitalRead(JOY_DOWN) == LOW && directionY != -1) { // Down
      directionX = 0;
      directionY = 1;
    } else if (digitalRead(JOY_LEFT) == LOW && directionX != 1) { // Left
      directionX = -1;
      directionY = 0;
    } else if (digitalRead(JOY_RIGHT) == LOW && directionX != -1) { // Right
      directionX = 1;
      directionY = 0;
    }

    // Move snake
    int prevX[MAX_SNAKE_LENGTH], prevY[MAX_SNAKE_LENGTH];
    for (int i = 0; i < snakeLength; i++) {
      prevX[i] = snakeX[i];
      prevY[i] = snakeY[i];
    }

    snakeX[0] += directionX * BLOCK_SIZE;
    snakeY[0] += directionY * BLOCK_SIZE;

    for (int i = 1; i < snakeLength; i++) {
      snakeX[i] = prevX[i - 1];
      snakeY[i] = prevY[i - 1];
    }

    // Check collisions
    if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 0 || snakeY[0] >= SCREEN_HEIGHT) {
      gameOver = true;
    }

    for (int i = 1; i < snakeLength; i++) {
      if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i]) {
        gameOver = true;
        break;
      }
    }

    // Check if snake eats food
    if (snakeX[0] == foodX && snakeY[0] == foodY) {
      snakeLength++;
      if (snakeLength >= MAX_SNAKE_LENGTH) snakeLength = MAX_SNAKE_LENGTH;
      foodX = random(0, SCREEN_WIDTH / BLOCK_SIZE) * BLOCK_SIZE;
      foodY = random(0, SCREEN_HEIGHT / BLOCK_SIZE) * BLOCK_SIZE;
    }

    // Draw game
    u8g2.clearBuffer();

    // Draw Snake
    for (int i = 0; i < snakeLength; i++) {
      u8g2.drawBox(snakeX[i], snakeY[i], BLOCK_SIZE, BLOCK_SIZE);
    }

    // Draw Food
    u8g2.drawBox(foodX, foodY, BLOCK_SIZE, BLOCK_SIZE);

    // Draw score
    char scoreStr[16];
    sprintf(scoreStr, "Score: %d", snakeLength - 5);
    u8g2.drawStr(0, SCREEN_HEIGHT - 10, scoreStr);

    if (gameOver) {
      u8g2.setDrawColor(1);
      u8g2.drawStr(SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, "Game Over!");
    }

    u8g2.sendBuffer();

    lastMoveTime = currentTime;

    if (gameOver) {
      delay(2000); // Show game over for 2 seconds
      // Reset game
      snakeLength = 5;
      int startX = SCREEN_WIDTH / 2 / BLOCK_SIZE * BLOCK_SIZE;
      int startY = SCREEN_HEIGHT / 2 / BLOCK_SIZE * BLOCK_SIZE;
      for (int i = 0; i < snakeLength; i++) {
        snakeX[i] = startX - i * BLOCK_SIZE;
        snakeY[i] = startY;
      }
      foodX = random(0, SCREEN_WIDTH / BLOCK_SIZE) * BLOCK_SIZE;
      foodY = random(0, SCREEN_HEIGHT / BLOCK_SIZE) * BLOCK_SIZE;
      directionX = 1;
      directionY = 0;
      gameOver = false;
    }
  }
}

void startscreen(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_14b_tf);	
  u8g2.drawStr(10,30,"MOCHInator V1");	
  u8g2.sendBuffer();
  delay(1000);
  
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 128, 64, myBitmap);
  u8g2.sendBuffer();
  playSong(starwarsMelody, starwarsDurations);
  // delay(3000);
  
  // u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_squeezed_b7_tr);
  u8g2.drawStr(90, 20, "Press");
  u8g2.drawStr(95, 30, "SW3");
  u8g2.drawStr(100, 40, "to");
  u8g2.drawStr(85, 50, "continue");
  // u8g2.drawXBMP(0, 0, 128, 64, myBitmap);
  u8g2.sendBuffer();
  delay(300);
}

void playSong(int audio[], int audioDuration[]) {
  for (int thisNote = 0; thisNote < noteSize; thisNote++) {
    if (digitalRead(buttonBPin) == LOW) {
      noTone(1);             // Stop sound immediately
      modeState = 0;         // Go back to menu or initial state
      return;
    }

    int noteDuration = 1000 / audioDuration[thisNote];
    tone(1, audio[thisNote], noteDuration);

    // Wait while checking buttonB to allow real-time exit
    unsigned long startTime = millis();
    while (millis() - startTime < noteDuration) {
      if (digitalRead(buttonBPin) == LOW) {
        noTone(1);
        modeState = 0;
        return;
      }
    }

    noTone(1); // Ensure the tone is turned off
    delay(50); // Optional short gap between notes
  }
}

void playSongWithScrollingText(const String &text, int maxLines, int audio[], int audioDuration[]) {
  int linesPerScreen = 3;
  unsigned long scrollDelay = 150;
  unsigned long lastScroll = 0;

  for (int thisNote = 0; thisNote < noteSize; thisNote++) {
    // Check if button B is pressed to exit
    if (digitalRead(buttonBPin) == LOW) {
      noTone(1);
      modeState = 0;
      delay(1000);
      return;
    }

    // Scroll text with joystick
    if (millis() - lastScroll > scrollDelay) {
      if (digitalRead(jsupPin) == LOW && scrollIndex > 0) {
        scrollIndex--;
        lastScroll = millis();
      }
      if (digitalRead(jsdownPin) == LOW && scrollIndex < maxLines - linesPerScreen) {
        scrollIndex++;
        lastScroll = millis();
      }
    }

    // Display text lines
    u8g2.clearBuffer();
    for (int i = 0; i < linesPerScreen; i++) {
      int lineNumber = scrollIndex + i;
      int charStart = lineNumber * 21;
      unsigned int charEnd = min((unsigned int)((lineNumber + 1) * 21), text.length());
      if (charStart < text.length()) {
        String line = text.substring(charStart, charEnd);
        u8g2.drawStr(0, 15 + (i * 20), line.c_str());
      }
    }
    u8g2.sendBuffer();

    // Play the note
    int noteDuration = 1000 / audioDuration[thisNote];
    tone(1, audio[thisNote], noteDuration);

    // Wait with real-time button check
    unsigned long startTime = millis();
    while (millis() - startTime < noteDuration) {
      if (digitalRead(buttonBPin) == LOW) {
        noTone(1);
        modeState = 0;
        return;
      }
    }

    noTone(1);
    delay(50); // Optional gap between notes
  }
}


void scrollTextWithJoystick(const String text, int lineHeight = 14) {
  static int scrollOffset = 0;
  static unsigned long lastScroll = 0;
  const int scrollDelay = 150;

  String str(text);
  int totalLines = ceil((float)str.length() / 21.0);  // ~21 chars per line
  int maxOffset = max(0, totalLines - 4); // Show max 4 lines

  // Read joystick
  bool jsup = digitalRead(jsupPin) == LOW;
  bool jsdown = digitalRead(jsdownPin) == LOW;
  bool buttonB = digitalRead(buttonBPin) == LOW;

  // Handle scrolling
  if (millis() - lastScroll > scrollDelay) {
    if (jsup && scrollOffset > 0) {
      scrollOffset--;
      lastScroll = millis();
    } else if (jsdown && scrollOffset < maxOffset) {
      scrollOffset++;
      lastScroll = millis();
    }
  }

  // Exit on Button B
  if (buttonB) {
    scrollOffset = 0;
    modeState = 0; // Replace with your menu state value
    return;
  }

  // Draw text
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x13_tf);

  for (int i = 0; i < 4; i++) {
    int lineIndex = i + scrollOffset;
    if (lineIndex < totalLines) {
      unsigned int start = lineIndex * 21;
      unsigned int end = min((unsigned int)str.length(), (unsigned int)((lineIndex + 1) * 21));
      String lineText = str.substring(start, end);
      u8g2.drawStr(0, (i + 1) * lineHeight - 2, lineText.c_str());
    }
  }

  u8g2.sendBuffer();
}

void setup(void) {
  u8g2.begin();
  pixels.begin();
  pinMode(jsupPin, INPUT_PULLUP);
  pinMode(jsdownPin, INPUT_PULLUP);
  pinMode(jsrightPin, INPUT_PULLUP);
  pinMode(jsleftPin, INPUT_PULLUP);
  pinMode(jsswitchPin, INPUT_PULLUP);
  pinMode(buttonAPin, INPUT_PULLUP);
  pinMode(buttonBPin, INPUT_PULLUP);
  pinMode(reedPin, INPUT_PULLUP);
  u8g2.setFont(u8g2_font_6x13_tf);
  noteSize = sizeof(BdayNoteDurations) / sizeof(int);
  startSCR = 1;

  // Initialize Snake game
  snakeLength = 5;
  int startX = SCREEN_WIDTH / 2 / BLOCK_SIZE * BLOCK_SIZE;
  int startY = SCREEN_HEIGHT / 2 / BLOCK_SIZE * BLOCK_SIZE;
  for (int i = 0; i < snakeLength; i++) {
    snakeX[i] = startX - i * BLOCK_SIZE;
    snakeY[i] = startY;
  }
  foodX = random(0, SCREEN_WIDTH / BLOCK_SIZE) * BLOCK_SIZE;
  foodY = random(0, SCREEN_HEIGHT / BLOCK_SIZE) * BLOCK_SIZE;
  lastMoveTime = millis();
}

void loop(void) {
  if (startSCR) {
    startscreen();

    // Wait until any button is pressed
    while (digitalRead(buttonAPin) == HIGH && digitalRead(buttonBPin) == HIGH) {
      delay(10);
    }

    // Optional: wait until the button is released again
    while (digitalRead(buttonAPin) == LOW || digitalRead(buttonBPin) == LOW) {
      delay(10);
    }

    // Disable start screen for rest of session
    startSCR = 0;
    u8g2.clearBuffer();
    u8g2.drawXBMP(0, 0, 128, 64, speech_bubble);
    u8g2.sendBuffer();
    delay(2000);
  }
  
  unsigned long timer = millis();

  switch (modeState) {
    case 0: { // Main menu
      jsupVal = digitalRead(jsupPin);
      jsdownVal = digitalRead(jsdownPin);
      jsswitchVal = digitalRead(jsswitchPin);
      buttonAVal = digitalRead(buttonAPin);
      buttonBVal = digitalRead(buttonBPin);

      if (jsupVal == LOW && menuIndex > 0) {
        menuIndex--;
        delay(200);
      } else if (jsdownVal == LOW && menuIndex < menuLength - 1) {
        menuIndex++;
        delay(200);
      }

      u8g2.clearBuffer();
      u8g2.setFont(u8g2_font_6x13_tf);
      u8g2.drawStr(10, 10, "== MENU ==");

      for (int i = 0; i < menuLength; i++) {
        if (i == menuIndex) {
          u8g2.setDrawColor(1);
          u8g2.drawBox(2, 11 + i * 20, 124, 18);
          u8g2.setDrawColor(0);
        } else {
          u8g2.setDrawColor(1);
        }
        u8g2.drawStr(3, 24 + i * 20, menuItems[i]);
      }

      u8g2.setDrawColor(1);
      u8g2.sendBuffer();
      if (buttonAVal == LOW) {
        if (menuIndex == 0) {
          modeState = 1;  // Start Test
          scrollIndex = 0;
          randForJ = random(0, 291);
        } else if (menuIndex == 1) {
          // Settings placeholder
          modeState = 2;
        } else if (menuIndex == 2) {
          // About placeholder
          modeState = 3;
        }
        delay(200);
      }
      break;
    }
    case 1: {
      
      if (randForJ <= 100){
        totalLines = (messages[randForJ].length() + 20) / 21;
        playSongWithScrollingText(messages[randForJ], totalLines, harryPotterMelody, harryPotterDurations);
      }
      else {
        totalLines = (messages[randForJ].length() + 20) / 21;
        playSongWithScrollingText(messages[randForJ], totalLines, rickrollMelody, rickrollDurations);
      }
      break;
    }
    case 2: //any game here
      runSnakeGame(); // Run the Snake game
      // Check for exit (button B to return to menu)
      if (digitalRead(buttonBPin) == LOW) {
        modeState = 0; // Return to menu
        delay(300);    // Debounce
      }
      break;
    case 3: //birthday card
      u8g2.clearBuffer();
      u8g2.drawXBMP(0, 0, 128, 64, bdayCat);
      u8g2.setFont(u8g2_font_amstrad_cpc_extended_8f);
      u8g2.drawStr(10, 10, "Happy");
      u8g2.drawStr(76, 10, "Happy");
      u8g2.setFont(u8g2_font_squeezed_b7_tr);
      u8g2.drawStr(15, 54, "Birth");
      u8g2.drawStr(90, 54, "Day");
      u8g2.sendBuffer();
      playSong(BdayMelody, BdayNoteDurations);
      delay(5000);
      modeState = 0;
      
      break;
  }
  
}