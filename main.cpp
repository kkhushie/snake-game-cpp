#include <iostream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()
#include <conio.h>   // for _kbhit() and _getch()
#include <windows.h> // for Sleep()
using namespace std;

const int width = 20;
const int height = 10;

int main() {
    srand(time(0)); // Seed random generator

    int foodX = rand() % width;
    int foodY = rand() % height;
    int score = 0;

    int headX = width / 2;
    int headY = height / 2;

    char dir = ' ';
    bool gameOver = false;

    while (!gameOver) {
        // Draw top border
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        // Draw side walls, empty space, and food
        for (int i = 0; i < height; i++) {
            cout << "#";
            for (int j = 0; j < width; j++) {
                if (i == headY && j == headX) cout << "O";
                else if (i == foodY && j == foodX) cout << "@";
                else cout << " ";
            }
            cout << "#" << endl;
        }

        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        cout << "\nScore: " << score << endl;

        if (_kbhit()) {
            dir = _getch();
            if (dir == 'q') break;
        }

        switch (dir) {
            case 'w': headY--; break;
            case 's': headY++; break;
            case 'a': headX--; break;
            case 'd': headX++; break;
        }

        if (headX < 0 || headX >= width || headY < 0 || headY >= height) {
            gameOver = true;
        }

        if (headX == foodX && headY == foodY) {
            score += 10;
            foodX = rand() % width;
            foodY = rand() % height;
        }

        // Speed increases every 20 points
        int speed = max(50, 150 - (score / 2));
        Sleep(speed);

        system("cls");
    }

    cout << "Game Over! Final Score: " << score << endl;
    return 0;
}
