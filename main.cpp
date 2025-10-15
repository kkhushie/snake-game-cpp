#include <iostream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()
#include <conio.h>   // for _kbhit() and _getch()
using namespace std;

const int width = 20;
const int height = 10;

int main() {
    srand(time(0)); // Seed random generator

    // Generate random food position inside the box
    int foodX = rand() % width;
    int foodY = rand() % height;
int score = 0; 

     // Snake head starting position
    int headX = width / 2;
    int headY = height / 2;

    char dir = ' '; // initial direction
    bool gameOver = false;

    while(!gameOver) {
    // Draw top border
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << endl;

    // Draw side walls, empty space, and food
    for (int i = 0; i < height; i++) {
        cout << "#"; // left wall
        for (int j = 0; j < width; j++) {
                if (i == foodY && j == foodX) {
                cout << "@";
                score += 10; 
            }
            else
                cout << " ";
        }
        cout << "#"; // right wall
        cout << endl;
    }

    // Draw bottom border
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << endl;

    cout << "\nFood spawned at (" << foodX << ", " << foodY << ") 🍎" << endl;
cout << "Current Score: " << score << " points!" << endl;  
         // Input
        if (_kbhit()) {
            dir = _getch(); // w/a/s/d for movement, q to quit
            if (dir == 'q') break;
        }

        // Move snake head
        switch (dir) {
            case 'w': headY--; break;
            case 's': headY++; break;
            case 'a': headX--; break;
            case 'd': headX++; break;
        }

        // Check wall collision
        if (headX < 0 || headX >= width || headY < 0 || headY >= height) {
            gameOver = true;
        }

        // Check food collision
        if (headX == foodX && headY == foodY) {
            score += 10;
            foodX = rand() % width;   // respawn food
            foodY = rand() % height;
        }

        system("cls"); // clear screen for next frame
    }

    cout << "Game Over! Final Score: " << score << endl;
    return 0;
}
