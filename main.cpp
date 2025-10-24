#include <iostream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()
#include <conio.h>   // for _kbhit() and _getch()
#include <windows.h> // for Sleep()
#include <vector>    // for snake tail
#include <algorithm> // for max()
#include <fstream> //  for file handling ( saving high score )
using namespace std;

const int width = 20;
const int height = 10;

int main() {
    srand(time(0)); // Seed random generator

    int foodX = rand() % width;
    int foodY = rand() % height;
    int score = 0;

    // --- High Score Variable ---
    int highScore = 0; // Tracks the highest score in the current session

     // Load high score from file if exists
    ifstream inFile("highscore.txt");
    if (inFile.is_open()) {
        inFile >> highScore;
        inFile.close();
    }
    // ----------------------------------------

    int headX = width / 2;
    int headY = height / 2;

    // Snake body variables
    vector<int> tailX;
    vector<int> tailY;
    int tailLength = 0;

    char dir = ' ';
    bool gameOver = false;

    while (!gameOver) {
        // Draw top border
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        // Draw game area
        for (int i = 0; i < height; i++) {
            cout << "#";
            for (int j = 0; j < width; j++) {
                // Check if current position is snake head
                if (i == headY && j == headX) {
                    cout << "O";
                }
                // Check if current position is food
                else if (i == foodY && j == foodX) {
                    cout << "@";
                }
                else {
                    // Check if current position is part of snake tail
                    bool isTail = false;
                    for (int k = 0; k < tailLength; k++) {
                        if (tailX[k] == j && tailY[k] == i) {
                            cout << "o";
                            isTail = true;
                            break;
                        }
                    }
                    // If not head, food, or tail, print empty space
                    if (!isTail) {
                        cout << " ";
                    }
                }
            }
            cout << "#" << endl;
        }

        // Draw bottom border
        for (int i = 0; i < width + 2; i++) cout << "#";
        cout << endl;

        // --- Display High Score ---
        cout << "\nScore: " << score << " | Length: " << tailLength + 1 << " | High Score: " << highScore << endl;
        cout << "Controls: WASD to move, Q to quit" << endl;

        // Input handling
        if (_kbhit()) {
            dir = _getch();
            if (dir == 'q') break;
        }

        // Move snake tail
        if (tailLength > 0) {
            // Add current head position to the beginning of tail
            tailX.insert(tailX.begin(), headX);
            tailY.insert(tailY.begin(), headY);
            
            // Remove excess tail segments to maintain correct length
            while (tailX.size() > tailLength) {
                tailX.pop_back();
                tailY.pop_back();
            }
        }

        // Move snake head based on direction
        switch (dir) {
            case 'w': headY--; break;
            case 's': headY++; break;
            case 'a': headX--; break;
            case 'd': headX++; break;
        }

        // Wall collision detection
        if (headX < 0 || headX >= width || headY < 0 || headY >= height) {
            gameOver = true;
        }

        // Self collision detection (snake hits its own tail)
        for (int i = 0; i < tailLength; i++) {
            if (tailX[i] == headX && tailY[i] == headY) {
                gameOver = true;
                break;
            }
        }

        // Food collection
        if (headX == foodX && headY == foodY) {
            score += 10;
            tailLength++;  // Snake grows when eating food
            
            // Generate new food position
            foodX = rand() % width;
            foodY = rand() % height;
            
            // Make sure food doesn't spawn on snake
            bool foodOnSnake;
            do {
                foodOnSnake = false;
                if (foodX == headX && foodY == headY) {
                    foodOnSnake = true;
                }
                for (int i = 0; i < tailLength; i++) {
                    if (foodX == tailX[i] && foodY == tailY[i]) {
                        foodOnSnake = true;
                        break;
                    }
                }
                if (foodOnSnake) {
                    foodX = rand() % width;
                    foodY = rand() % height;
                }
            } while (foodOnSnake);
        }

        // Game speed (increases with score)
        int speed = max(50, 150 - (score / 2));
        Sleep(speed);

        system("cls");
    }

    //  Save updated high score to file
    if (score > highScore) {
        highScore = score;
        ofstream outFile("highscore.txt");
        if (outFile.is_open()) {
            outFile << highScore;
            outFile.close();
        }
    }
   
    
    // --- MODIFIED: Final Score Message ---
    cout << "Game Over! Final Score: " << score << " | High Score: " << highScore << " | Final Length: " << tailLength + 1 << endl;
    
    cout << "Thanks for playing!" << endl;
    
    return 0;
}
