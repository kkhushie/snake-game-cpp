#include<iostream>
using std namespace;

const int width = 20;
const int height = 10;

int main(){

   // Draw top border
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << endl;

   // Draw side walls with empty space inside
    for (int i = 0; i < height; i++) {
        cout << "#"; // left wall
        for (int j = 0; j < width; j++) cout << " ";
        cout << "#"; // right wall
        cout << endl;
    }

   // Draw bottom border
    for (int i = 0; i < width + 2; i++) cout << "#";
    cout << endl;
   
return 0;
}
