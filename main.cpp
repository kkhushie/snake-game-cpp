#include <iostream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()
#include <vector>    // for snake tail
#include <algorithm> // for max()
#include <fstream>   // for file handling (saving high score)

#ifdef _WIN32
#include <conio.h>   // for _kbhit() and _getch() on Windows
#include <windows.h> // for Sleep() on Windows
#else
#include <unistd.h>  // for usleep() on Linux/Mac
#include <termios.h> // for terminal settings on Linux/Mac
#include <fcntl.h>   // for file control on Linux/Mac
#endif

using namespace std;

const int width = 20;
const int height = 10;

// Cross-platform keyboard input function - RENAMED to avoid conflict
bool crossKbhit() {
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return true;
    }

    return false;
#endif
}

// Cross-platform get character function - RENAMED to avoid conflict
char crossGetch() {
#ifdef _WIN32
    return _getch();
#else
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
#endif
}

// Cross-platform sleep function
void crossSleep(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Cross-platform clear screen function
void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Function to set cursor position (reduces blinking)
void setCursorPosition(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    cout << "\033[" << y << ";" << x << "H";
#endif
}

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

    int headX = width / 2;
    int headY = height / 2;

    // Snake body variables
    vector<int> tailX;
    vector<int> tailY;
    int tailLength = 0;

    char dir = ' ';
    bool gameOver = false;
    bool isPaused = false; // NEW: Pause state

    // Clear screen only once at start
    clearScreen();

    while (!gameOver) {
        // Move cursor to top instead of clearing screen every time
        setCursorPosition(0, 0);

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
        cout << "Controls: WASD to move, P to pause, Q to quit" << endl;
        
        // NEW: Show pause message when game is paused
        if (isPaused) {
            cout << "*** GAME PAUSED - Press P to resume ***" << endl;
        } else {
            cout << "                                        " << endl; // Clear pause line
        }

        // Input handling - USING RENAMED FUNCTIONS
        if (crossKbhit()) {
            char input = crossGetch();
            
            // NEW: Pause/Resume functionality
            if (input == 'p' || input == 'P') {
                isPaused = !isPaused;
                // Don't print messages to avoid screen jump
            } else if (input == 'q' || input == 'Q') {
                break;
            } else if (!isPaused) { // Only process movement keys when not paused
                // Prevent 180-degree turns
                if (!((dir == 'w' || dir == 'W') && (input == 's' || input == 'S')) &&
                    !((dir == 's' || dir == 'S') && (input == 'w' || input == 'W')) &&
                    !((dir == 'a' || dir == 'A') && (input == 'd' || input == 'D')) &&
                    !((dir == 'd' || dir == 'D') && (input == 'a' || input == 'A'))) {
                    dir = input;
                }
            }
        }

        // Skip game logic if paused
        if (isPaused) {
            crossSleep(100);
            continue;
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
            case 'w': case 'W': headY--; break;
            case 's': case 'S': headY++; break;
            case 'a': case 'A': headX--; break;
            case 'd': case 'D': headX++; break;
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
        crossSleep(speed);
    }

    // Clear screen for final message
    clearScreen();

    // Save updated high score to file
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
    
    // Wait for key press before exiting
    cout << "Press any key to exit...";
    crossGetch();
    
    return 0;
}
