#ifndef GAME_2048_H
#define GAME_2048_H

#include <Arduino.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

// Game state and UI
extern char modeState;  // 0 = menu, 1 = game



// Pins - Declare as extern
extern const int jsupPin;
extern const int jsdownPin;
extern const int jsleftPin;
extern const int jsrightPin;
extern const int buttonBPin;


// Define board
const int SIZE = 4;
int board[SIZE][SIZE] = {0};
bool gameOver = false;

// Move flags and timing
unsigned long lastMoveTime = 0;
const unsigned long moveDelay = 200;  // ms
// Utility functions
void drawBoard() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            char buf[6];
            if (board[y][x] != 0)
                sprintf(buf, "%4d", board[y][x]);
            else
                sprintf(buf, "    ");
            u8g2.drawStr(x * 32 + 2, y * 16 + 14, buf);
        }
    }
    u8g2.sendBuffer();
}

void addRandomTile() {
    int emptyCount = 0;
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            if (board[y][x] == 0) emptyCount++;

    if (emptyCount == 0) return;

    int r = random(emptyCount);
    for (int y = 0; y < SIZE; y++) {
        for (int x = 0; x < SIZE; x++) {
            if (board[y][x] == 0) {
                if (r == 0) {
                    board[y][x] = (random(10) < 9) ? 2 : 4;
                    return;
                }
                r--;
            }
        }
    }
}

void initBoard() {
    for (int y = 0; y < SIZE; y++)
        for (int x = 0; x < SIZE; x++)
            board[y][x] = 0;
    addRandomTile();
    addRandomTile();
    gameOver = false;
}

bool slideAndCombineRow(int row[SIZE]) {
    bool changed = false;
    int last = -1, index = 0;
    int newRow[SIZE] = {0};

    for (int i = 0; i < SIZE; i++) {
        if (row[i] == 0) continue;
        if (last == -1) {
            last = row[i];
        } else {
            if (row[i] == last) {
                newRow[index++] = last * 2;
                last = -1;
                changed = true;
            } else {
                newRow[index++] = last;
                last = row[i];
            }
        }
    }
    if (last != -1) newRow[index] = last;

    for (int i = 0; i < SIZE; i++) {
        if (row[i] != newRow[i]) changed = true;
        row[i] = newRow[i];
    }
    return changed;
}

bool moveLeft() {
    bool moved = false;
    for (int y = 0; y < SIZE; y++)
        moved |= slideAndCombineRow(board[y]);
    return moved;
}

bool moveRight() {
    bool moved = false;
    for (int y = 0; y < SIZE; y++) {
        int row[SIZE];
        for (int x = 0; x < SIZE; x++) row[x] = board[y][SIZE - 1 - x];
        moved |= slideAndCombineRow(row);
        for (int x = 0; x < SIZE; x++) board[y][SIZE - 1 - x] = row[x];
    }
    return moved;
}

bool moveUp() {
    bool moved = false;
    for (int x = 0; x < SIZE; x++) {
        int col[SIZE];
        for (int y = 0; y < SIZE; y++) col[y] = board[y][x];
        moved |= slideAndCombineRow(col);
        for (int y = 0; y < SIZE; y++) board[y][x] = col[y];
    }
    return moved;
}

bool moveDown() {
    bool moved = false;
    for (int x = 0; x < SIZE; x++) {
        int col[SIZE];
        for (int y = 0; y < SIZE; y++) col[y] = board[SIZE - 1 - y][x];
        moved |= slideAndCombineRow(col);
        for (int y = 0; y < SIZE; y++) board[SIZE - 1 - y][x] = col[y];
    }
    return moved;
}

void handle2048() {
    if (gameOver) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(10, 30, "Game Over!");
        u8g2.sendBuffer();
        delay(2000);
        modeState = 0;
        return;
    }

    drawBoard();

    if (millis() - lastMoveTime > moveDelay) {
        bool moved = false;
        if (digitalRead(jsupPin) == LOW) {
            moved = moveUp();
        } else if (digitalRead(jsdownPin) == LOW) {
            moved = moveDown();
        } else if (digitalRead(jsleftPin) == LOW) {
            moved = moveLeft();
        } else if (digitalRead(jsrightPin) == LOW) {
            moved = moveRight();
        }

        if (moved) {
            addRandomTile();
            drawBoard();
            lastMoveTime = millis();
        }
    }

    if (digitalRead(buttonBPin) == LOW) {
        modeState = 0;
        delay(300);
    }
}

#endif