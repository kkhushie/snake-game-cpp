#include <iostream>
#include <conio.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>
using namespace std;

// Board dimensions
const int width = 25;
const int height = 15;

// Global variables
int x, y, fruitX, fruitY, bombX, bombY;
int tailX[100], tailY[100];
int nTail;
int score, highScore = 0;
bool gameOver, bombHit = false;
string playerName;
bool firstTime = true;

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
Direction dir;
int delayTime = 120;
HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

void setCursorPos(int X, int Y) {
    COORD pos = { (SHORT)X, (SHORT)Y };
    SetConsoleCursorPosition(hOut, pos);
}

void HideCursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void generateObjects() {
    fruitX = rand() % width;
    fruitY = rand() % height;
    bombX = rand() % width;
    bombY = rand() % height;
    if (bombX == fruitX && bombY == fruitY)
        bombX = (bombX + 4) % width;
}

void Setup() {
    srand((unsigned)time(0));
    gameOver = false;
    bombHit = false;
    dir = STOP;
    x = width / 2;
    y = height / 2;
    score = 0;
    nTail = 0;
    generateObjects();
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    HideCursor();
}

void DrawBorderLine() {
    for (int i = 0; i < width + 2; i++)
        cout << "💎";
    cout << "\n";
}

void Draw() {
    setCursorPos(0, 0);
    cout << "🎮 Player: " << playerName 
         << "    ⭐ Score: " << score 
         << "    🏆 High Score: " << highScore << "\n";
    DrawBorderLine();

    for (int i = 0; i < height; i++) {
        cout << "💎";
        for (int j = 0; j < width; j++) {
            if (i == y && j == x)
                cout << "😶";
            else if (i == fruitY && j == fruitX)
                cout << "🍎";
            else if (i == bombY && j == bombX)
                cout << "💣";
            else {
                bool printTail = false;
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        cout << "🟡";
                        printTail = true;
                        break;
                    }
                }
                if (!printTail)
                    cout << "  ";
            }
        }
        cout << "💎\n";
    }

    DrawBorderLine();
}

void Input() {
    if (_kbhit()) {
        switch (_getch()) {
        case 'a': case 'A': dir = LEFT; break;
        case 'd': case 'D': dir = RIGHT; break;
        case 'w': case 'W': dir = UP; break;
        case 's': case 'S': dir = DOWN; break;
        case 'x': case 'X': gameOver = true; break;
        }
    }
}

void Logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;

    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }

    switch (dir) {
    case LEFT:  x--; break;
    case RIGHT: x++; break;
    case UP:    y--; break;
    case DOWN:  y++; break;
    default: break;
    }

    // Boundary check (game over)
    if (x >= width || x < 0 || y >= height || y < 0)
        gameOver = true;

    // Tail collision
    for (int i = 0; i < nTail; i++)
        if (tailX[i] == x && tailY[i] == y)
            gameOver = true;

    // Fruit eaten
    if (x == fruitX && y == fruitY) {
        score += 10;
        nTail++;
        generateObjects();
        if (score > highScore) highScore = score;
    }

    // Bomb hit
    if (x == bombX && y == bombY) {
        bombHit = true;
        gameOver = true;
    }
}

void GameOverMessage() {
    system("cls");
    if (bombHit) {
        cout << "\n💣💥  Uh oh! You just hugged a bomb, " << playerName << "! 😅\n";
        cout << "Next time, maybe avoid the explosive stuff, okay? 😂\n\n";
    } else {
        cout << "\n💀 Oops! " << playerName << ", your snake crashed. \n";
        cout << "That’s what we call a *fatal bite*! 😆\n\n";
    }
    cout << "🎯 Final Score: " << score << "\n";
    cout << "🏆 High Score: " << highScore << "\n\n";
}

void ChooseLevel() {
    cout << "\n Choose difficulty:\n";
    cout << "   1️ Easy   (Relaxed)\n";
    cout << "   2️ Medium (Classic)\n";
    cout << "   3️  Hard   (Blink and Die)\n";
    cout << " Enter choice: ";
    int choice;
    cin >> choice;

    if (choice == 1) delayTime = 180;
    else if (choice == 2) delayTime = 120;
    else if (choice == 3) delayTime = 70;
    else delayTime = 120;
}

void ShowIntro() {
    system("cls");
    SetConsoleOutputCP(CP_UTF8);
    if (firstTime) {
        cout << "\n💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎\n";
        cout << "          WELCOME TO SNAKE QUEST \n";
        cout << "💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎\n\n";
        cout << "👤 Enter your name: ";
        cin >> playerName;
        firstTime = false;
    } else {
        cout << "\n Welcome back, " << playerName << "! Ready for another round?\n";
    }

    ChooseLevel();

    cout << "\n🚀 Setting up your arena...\n";
    this_thread::sleep_for(chrono::milliseconds(1500));
}

void PlayGame() {
    Setup();
    while (!gameOver) {
        Draw();
        Input();
        Logic();
        this_thread::sleep_for(chrono::milliseconds(delayTime));
    }
    GameOverMessage();
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    bool playAgain = true;
    string choice;

    while (playAgain) {
        ShowIntro();
        PlayGame();

        cout << "🔁 Play again, " << playerName << "? (y/n): ";
        cin >> choice;

        if (choice != "y" && choice != "Y") {
            playAgain = false;
            cout << "\n✨ Thanks for playing Snake Quest, " << playerName << "! 💎\n";
            cout << "🏁 Final High Score: " << highScore << "\n";
            cout << "Catch you later! \n";
            system("pause");
        }
    }
    return 0;
}

