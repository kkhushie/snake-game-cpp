#include <iostream>
#include <cstdlib>   // for rand(), srand()
#include <ctime>     // for time()
using namespace std;

const int width = 20;
const int height = 10;

int main() {
    srand(time(0)); // Seed random generator

    // Generate random food position inside the box
    int foodX = rand() % width;
    int foodY = rand() % height;
int score = 0; 
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
    return 0;
}
