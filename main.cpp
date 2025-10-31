// required header file
#include <conio.h>
#include <iostream>
#include <windows.h>
using namespace std;

// height and width of the boundary
const int width = 80;
const int height = 20;

// Snake head coordinates of snake (x-axis, y-axis)
int x, y;
// Food coordinates
int fruitCordX, fruitCordY;
// variable to store the score of the player
int playerScore;
// Arrays to store the coordinates of snake tail (x-axis, y-axis)
int snakeTailX[100], snakeTailY[100];
// variable to store the length of the snake's tail
int snakeTailLen;

// for storing snake's moving direction
enum snakesDirection { STOP = 0, LEFT, RIGHT, UP, DOWN };
snakesDirection sDir;

// boolean variable for checking game is over or not
bool isGameOver;

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
}

// Function for creating the game board & rendering
void GameRender(string playerName) {
    // Move cursor to 0,0 instead of clearing screen ❗
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hOut, pos);

    // Creating top walls with '-'
    for (int i = 0; i < width + 2; i++)
        cout << "-";
    cout << endl;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j <= width; j++) {
            if (j == 0 || j == width)
                cout << "|";  // side walls

            if (i == y && j == x)
                cout << "O"; // snake head
            else if (i == fruitCordY && j == fruitCordX)
                cout << "#"; // food
            else {
                bool printTail = false;
                for (int k = 0; k < snakeTailLen; k++) {
                    if (snakeTailX[k] == j && snakeTailY[k] == i) {
                        cout << "o";
                        printTail = true;
                    }
                }
                if (!printTail)
                    cout << " ";
            }
        }
        cout << endl;
    }

    // Creating bottom walls with '-'
    for (int i = 0; i < width + 2; i++)
        cout << "-";
    cout << endl;

    // Display player's score
    cout << playerName << "'s Score: " << playerScore << endl;
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

    // Wall collision
    if (x >= width || x < 0 || y >= height || y < 0)
        isGameOver = true;

    // Tail collision
    for (int i = 0; i < snakeTailLen; i++) {
        if (snakeTailX[i] == x && snakeTailY[i] == y)
            isGameOver = true;
    }

    // Food collision
    if (x == fruitCordX && y == fruitCordY) {
        playerScore += 10;
        fruitCordX = rand() % width;
        fruitCordY = rand() % height;
        snakeTailLen++;
    }
}

// Function to set the game difficulty level
int SetDifficulty() {
    int dfc, choice;
    cout << "\nSET DIFFICULTY\n1: Easy\n2: Medium\n3: Hard"
         << "\nNOTE: default = Medium\nChoose difficulty level: ";
    cin >> choice;

    switch (choice) {
    case 1: dfc = 150; break; // slower = easy
    case 2: dfc = 100; break;
    case 3: dfc = 50;  break; // faster = hard
    default: dfc = 100;
    }
    return dfc;
}

// Function to handle user input
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

// Main function / game loop
int main() {
    HideCursor();

    string playerName;
    cout << "Enter your name: ";
    cin >> playerName;

    int dfc = SetDifficulty();
    GameInit();

    while (!isGameOver) {
        GameRender(playerName);
        UserInput();
        UpdateGame();
        Sleep(dfc);
    }

    cout << "\nGAME OVER! Final Score: " << playerScore << endl;
    system("pause");
    return 0;
}
