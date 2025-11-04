#include <conio.h>
#include <iostream>
#include <windows.h>
#include <chrono>
#include <cstdio>
using namespace std;

// height and width of the boundary
const int width = 80;
const int height = 20;

// Snake head coordinates of snake (x-axis, y-axis)
int x, y;
// Food coordinates
int fruitCordX, fruitCordY;
// Bomb coordinates
int bombCordX, bombCordY;
// variable to store the score of the player
int playerScore;
// Arrays to store the coordinates of snake tail (x-axis, y-axis)
int snakeTailX[100], snakeTailY[100];
// variable to store the length of the snake's tail
int snakeTailLen;

int highScore = 0; //  High Score Variable

// for storing snake's moving direction
enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
snakesDirection sDir;

// boolean variable for checking game is over or not
bool isGameOver;

// Variables for bomb timing
bool bombActive = false;
auto lastBombTime = chrono::steady_clock::now();
const int BOMB_INTERVAL = 10; // seconds

// Load High Score from file
void LoadHighScore() {
    FILE* file = fopen("highscore.txt", "r");
    if (file) {
        fscanf(file, "%d", &highScore);
        fclose(file);
    }
}

//  Save High Score to file
void SaveHighScore() {
    FILE* file = fopen("highscore.txt", "w");
    if (file) {
        fprintf(file, "%d", highScore);
        fclose(file);
    }
}

// Hide blinking cursor
void HideCursor() {
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(out, &cursorInfo);
}

// Function to initialize game variables
void GameInit() {
    isGameOver = false;
    sDir = STOP;
    x = width / 2;
    y = height / 2;
    fruitCordX = rand() % width;
    fruitCordY = rand() % height;
    playerScore = 0;
    snakeTailLen = 0;
    bombActive = false;
    lastBombTime = chrono::steady_clock::now();

    LoadHighScore(); // Load high score
}

// Function for creating the game board & rendering
void GameRender(string playerName) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hOut, pos);

    for (int i = 0; i < width + 2; i++)
        cout << "-";
    cout << endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j <= width; j++) {
            if (j == 0 || j == width)
                cout << "|";

            if (i == y && j == x)
                cout << "\U0001F636";
            else if (i == fruitCordY && j == fruitCordX)
                cout << "\U0001F34E";
            else if (bombActive && i == bombCordY && j == bombCordX)
                cout << "\U0001F4A3";
            else {
                bool printTail = false;
                for (int k = 0; k < snakeTailLen; k++) {
                    if (snakeTailX[k] == j && snakeTailY[k] == i) {
                        cout << "\U0001F7E1";
                        printTail = true;
                    }
                }
                if (!printTail)
                    cout << " ";
            }
        }
        cout << endl;
    }

    for (int i = 0; i < width + 2; i++)
        cout << "-";
    cout << endl;

    cout << playerName << "'s Score: " << playerScore << endl;
    cout << "High Score: " << highScore << endl; //  Display High Score

    if (bombActive) {
        auto now = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(now - lastBombTime).count();
        cout << "Bomb Active! Time left: " << (BOMB_INTERVAL - elapsed) << "s" << endl;
    }
}

// Function to handle bomb spawning
void UpdateBomb() {
    auto now = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::seconds>(now - lastBombTime).count();
    
    if (!bombActive && elapsed >= BOMB_INTERVAL) {
        bombCordX = rand() % width;
        bombCordY = rand() % height;
        bombActive = true;
        lastBombTime = now;
    }
    else if (bombActive && elapsed >= BOMB_INTERVAL) {
        bombActive = false;
        lastBombTime = now;
    }
}

// Function for updating the game state
void UpdateGame() {
    int prevX = snakeTailX[0];
    int prevY = snakeTailY[0];
    int prev2X, prev2Y;
    snakeTailX[0] = x;
    snakeTailY[0] = y;

    for (int i = 1; i < snakeTailLen; i++) {
        prev2X = snakeTailX[i];
        prev2Y = snakeTailY[i];
        snakeTailX[i] = prevX;
        snakeTailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (sDir) {
    case LEFT:  x--; break;
    case RIGHT: x++; break;
    case UP:    y--; break;
    case DOWN:  y++; break;
    }

    if (x >= width || x < 0 || y >= height || y < 0)
        isGameOver = true;

    for (int i = 0; i < snakeTailLen; i++) {
        if (snakeTailX[i] == x && snakeTailY[i] == y)
            isGameOver = true;
    }

    if (x == fruitCordX && y == fruitCordY) {
        playerScore += 10;
        fruitCordX = rand() % width;
        fruitCordY = rand() % height;
        snakeTailLen++;
    }
    
    if (bombActive && x == bombCordX && y == bombCordY) {
        isGameOver = true;
        cout << "\nBOOM! You hit the bomb!" << endl;
    }
}

// Set difficulty
int SetDifficulty() {
    int dfc, choice;
    cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard"
         << "\nNOTE: default = Medium\nChoose difficulty level: ";
    cin >> choice;

    switch (choice) {
    case 1: dfc = 150; break;
    case 2: dfc = 100; break;
    case 3: dfc = 50;  break;
    default: dfc = 100;
    }
    return dfc;
}

// User input
void UserInput() {
    if (_kbhit()) {
        switch (_getch()) {
        case 'a': sDir = LEFT; break;
        case 'd': sDir = RIGHT; break;
        case 'w': sDir = UP; break;
        case 's': sDir = DOWN; break;
        case 'x': isGameOver = true; break;
        }
    }
}

int main() {
    HideCursor();
    SetConsoleOutputCP(CP_UTF8);

    string playerName;
    cout << "Enter your name: ";
    cin >> playerName;

    int dfc = SetDifficulty();
    GameInit();

    while (!isGameOver) {
        GameRender(playerName);
        UserInput();
        UpdateBomb();
        UpdateGame();
        Sleep(dfc);
    }

    //  Update and save high score
    if (playerScore > highScore) {
        highScore = playerScore;
        SaveHighScore();
        cout << "\n🎉 NEW HIGH SCORE! 🎉\n";
    }

    cout << "\nGAME OVER! Final Score: " << playerScore << endl;
    system("pause");
    return 0;
}
