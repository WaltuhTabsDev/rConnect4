/*******************************************************************************************
*
*   raylib gamejam template
*
*   Template originally created with raylib 4.5-dev, last time updated with raylib 5.0
*
*   Template licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2024 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "C:\raylib\raylib\src\raylib.h"

#if defined(PLATFORM_WEB)
    #define CUSTOM_MODAL_DIALOGS            // Force custom modal dialogs usage
    #include <emscripten/emscripten.h>      // Emscripten library - LLVM to JavaScript compiler
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // For rand() and srand() 
#include <string.h>                         // Required for: 
#include <time.h>                          // For time()
#define BOARD_WIDTH 7
#define BOARD_HEIGHT 6
#define CELL_SIZE 80
#define GAME_OVER_DELAY 1000 // Delay in milliseconds

typedef enum { EMPTY, PLAYER1, PLAYER2 } CellState;
typedef enum { TITLE_SCREEN, GAME_PLAY, GAME_OVER } GameState;
// Sound variables
Sound soundPlace; // Sound when a DVD is placed
Sound soundStart; // Sound when the game starts
CellState board[BOARD_HEIGHT][BOARD_WIDTH];
int currentPlayer = PLAYER1;
int winningPlayer = EMPTY;
int gameOverTimer = 0; // Timer for the game-over state

Color colors[8];
GameState gameState = TITLE_SCREEN;
bool isComputerMode = false; // Flag to determine if computer mode is active

void DrawBoard();
void ResetBoard();
bool PlaceDisk(int column);
bool CheckWin();
void DrawGameOverScreen();
void DrawTitleScreen();
int GetColumnFromMouse();
int GetRandomValidColumn();
void ComputerMove(); // Function for computer's turn
int GetBestMove(); // Function to determine the best move for AI
bool CheckWinningMove(int player, int column); // Check if placing in column will win
bool CanWinNextMove(int player); // Check if the player can win in next move
void UpdateDrawFrame(); // Function to handle updating and drawing

int main(void) {
    InitWindow(BOARD_WIDTH * CELL_SIZE, BOARD_HEIGHT * CELL_SIZE + 100, "Connect Four");
    SetTargetFPS(60);
    InitAudioDevice(); // Initialize audio device

    // Load sounds
    soundPlace = LoadSound("resources/soundplace.wav");
    soundStart = LoadSound("resources/soundstart.wav");
    
    
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // Define color palette
    colors[0] = (Color){ 255, 255, 255, 255 }; // White
    colors[1] = (Color){ 255, 0, 0, 255 };     // Red
    colors[2] = (Color){ 0, 0, 255, 255 };     // Blue
    colors[3] = (Color){ 255, 255, 0, 255 };   // Yellow
    colors[4] = (Color){ 0, 255, 0, 255 };     // Green
    colors[5] = (Color){ 255, 165, 0, 255 };   // Orange
    colors[6] = (Color){ 128, 0, 128, 255 };   // Purple
    colors[7] = (Color){ 0, 0, 0, 255 };       // Black

    ResetBoard();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1); // Set the main loop for the browser
#else
    while (!WindowShouldClose()) {
        UpdateDrawFrame(); // Call the update and draw function
    }
#endif
    UnloadSound(soundPlace);
    UnloadSound(soundStart);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void UpdateDrawFrame() {
    // Main game loop logic
    if (gameState == TITLE_SCREEN) {
        DrawTitleScreen();

        if (IsKeyPressed(KEY_ONE)) {
            PlaySound(soundStart);
            isComputerMode = true; // Set to computer mode
            gameState = GAME_PLAY;
        }
        if (IsKeyPressed(KEY_TWO)) {
            PlaySound(soundStart);
            isComputerMode = false; // Set to two-player mode
            gameState = GAME_PLAY;
        }
    } else if (gameState == GAME_PLAY) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int column = GetColumnFromMouse();
            if (column != -1) {
                if (PlaceDisk(column)) {
                    if (CheckWin()) {
                        winningPlayer = currentPlayer; // Set the winning player
                        gameState = GAME_OVER; // Move to game-over state
                        gameOverTimer = GAME_OVER_DELAY; // Start the timer for the game-over state
                    } else {
                        // Switch players after the move
                        currentPlayer = (currentPlayer == PLAYER1) ? PLAYER2 : PLAYER1;

                        // If in computer mode and it's the computer's turn
                        if (isComputerMode && currentPlayer == PLAYER2) {
                            ComputerMove(); // Computer makes its move
                            if (CheckWin()) {
                                winningPlayer = currentPlayer; // Set the winning player
                                gameState = GAME_OVER; // Move to game-over state
                                gameOverTimer = GAME_OVER_DELAY; // Start the timer for the game-over state
                            } else {
                                // Switch back to Player 1
                                currentPlayer = PLAYER1;
                            }
                        }
                    }
                }
            }
        }
    } else if (gameState == GAME_OVER) {
        // Decrement the timer
        if (gameOverTimer > 0) {
            gameOverTimer -= GetFrameTime() * 1000; // Convert frame time to milliseconds
        }

        // Handle mouse click to restart
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || gameOverTimer <= 0) {
            ResetBoard();
            gameState = TITLE_SCREEN; // Go back to title screen after resetting
            winningPlayer = EMPTY; // Reset the winning player
        }
    }

    BeginDrawing();
    ClearBackground(colors[7]);

    if (gameState == TITLE_SCREEN) {
        DrawTitleScreen();
    } else if (gameState == GAME_PLAY) {
        DrawBoard();
    } else if (gameState == GAME_OVER) {
        DrawGameOverScreen();
    }

    EndDrawing();
}

void DrawBoard() {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            Color color = colors[7]; // Default to empty color
            if (board[row][col] == PLAYER1) color = colors[1]; // Red
            else if (board[row][col] == PLAYER2) color = colors[2]; // Blue

            DrawRectangle(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
            DrawRectangleLines(col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE, colors[0]); // Outline
        }
    }

    // Draw column indicators
    for (int col = 0; col < BOARD_WIDTH; col++) {
        DrawText(TextFormat("%d", col + 1), col * CELL_SIZE + (CELL_SIZE / 2) - 10, BOARD_HEIGHT * CELL_SIZE + 10, 20, colors[0]);
    }
}

void ResetBoard() {
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            board[row][col] = EMPTY;
        }
    }
    currentPlayer = PLAYER1;
}

int GetColumnFromMouse() {
    Vector2 mousePos = GetMousePosition();
    int column = (int)(mousePos.x / CELL_SIZE);
    if (mousePos.y > BOARD_HEIGHT * CELL_SIZE) return -1; // Clicked below the board
    return (column >= 0 && column < BOARD_WIDTH) ? column : -1;
}

bool PlaceDisk(int column) {
    PlaySound(soundPlace);
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        if (board[row][column] == EMPTY) {
            board[row][column] = currentPlayer;
            return true;
        }
    }
    return false; // Column is full
}

bool CheckWin() {
    // Check horizontal, vertical, and diagonal for wins
    for (int row = 0; row < BOARD_HEIGHT; row++) {
        for (int col = 0; col < BOARD_WIDTH; col++) {
            if (board[row][col] != EMPTY) {
                // Check horizontal
                if (col + 3 < BOARD_WIDTH &&
                    board[row][col] == board[row][col + 1] &&
                    board[row][col] == board[row][col + 2] &&
                    board[row][col] == board[row][col + 3]) {
                    return true;
                }
                // Check vertical
                if (row + 3 < BOARD_HEIGHT &&
                    board[row][col] == board[row + 1][col] &&
                    board[row][col] == board[row + 2][col] &&
                    board[row][col] == board[row + 3][col]) {
                    return true;
                }
                // Check diagonal (bottom-left to top-right)
                if (row + 3 < BOARD_HEIGHT && col + 3 < BOARD_WIDTH &&
                    board[row][col] == board[row + 1][col + 1] &&
                    board[row][col] == board[row + 2][col + 2] &&
                    board[row][col] == board[row + 3][col + 3]) {
                    return true;
                }
                // Check diagonal (top-left to bottom-right)
                if (row - 3 >= 0 && col + 3 < BOARD_WIDTH &&
                    board[row][col] == board[row - 1][col + 1] &&
                    board[row][col] == board[row - 2][col + 2] &&
                    board[row][col] == board[row - 3][col + 3]) {
                    return true;
                }
            }
        }
    }
    return false; // No win detected
}

void DrawGameOverScreen() {
    const char *message = (winningPlayer == PLAYER1) ? "Player 1 Wins!" : "Player 2 Wins!";
    DrawText(message, (GetScreenWidth() / 2) - MeasureText(message, 40) / 2, (GetScreenHeight() / 2) - 20, 40, colors[0]);
}

void DrawTitleScreen() {
    DrawText("Connect Four", (GetScreenWidth() / 2) - MeasureText("Connect Four", 40) / 2, (GetScreenHeight() / 2) - 40, 40, colors[0]);
    DrawText("Press '1' for Computer Mode", (GetScreenWidth() / 2) - MeasureText("Press '1' for Computer Mode", 20) / 2, (GetScreenHeight() / 2) + 10, 20, colors[0]);
    DrawText("Press '2' for Two Player Mode", (GetScreenWidth() / 2) - MeasureText("Press '2' for Two Player Mode", 20) / 2, (GetScreenHeight() / 2) + 40, 20, colors[0]);
}

void ComputerMove() {
    int column = GetBestMove(); // Use AI to decide the best move
    if (column != -1) {
        PlaceDisk(column); // Place the disk in the chosen column
    }
}

int GetBestMove() {
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (CheckWinningMove(PLAYER2, col)) return col; // Block winning move
    }
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (CheckWinningMove(PLAYER1, col)) return col; // Take winning move
    }
    return GetRandomValidColumn(); // Fallback to random move
}

bool CheckWinningMove(int player, int column) {
    for (int row = BOARD_HEIGHT - 1; row >= 0; row--) {
        if (board[row][column] == EMPTY) {
            board[row][column] = player; // Simulate the move
            bool win = CheckWin();
            board[row][column] = EMPTY; // Undo the move
            return win;
        }
    }
    return false; // Column is full or no win
}

int GetRandomValidColumn() {
    int validColumns[BOARD_WIDTH];
    int count = 0;
    for (int col = 0; col < BOARD_WIDTH; col++) {
        if (board[0][col] == EMPTY) {
            validColumns[count++] = col; // Store valid columns
        }
    }
    return (count > 0) ? validColumns[rand() % count] : -1; // Return random valid column
}
