#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>
#include <atomic>
#include <vector>

// ===================================================
// Platform specifics
// ===================================================
#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>

  static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

  void setCursorPos(int X, int Y) {
      COORD pos = {(SHORT)X, (SHORT)Y};
      SetConsoleCursorPosition(hOut, pos);
  }
  void HideCursor() {
      CONSOLE_CURSOR_INFO info;
      GetConsoleCursorInfo(hOut, &info);
      info.bVisible = FALSE;
      SetConsoleCursorInfo(hOut, &info);
  }
  void ShowCursor() {
      CONSOLE_CURSOR_INFO info;
      GetConsoleCursorInfo(hOut, &info);
      info.bVisible = TRUE;
      SetConsoleCursorInfo(hOut, &info);
  }
  bool kbhitCheck() { return _kbhit(); }
  char getChar() { return _getch(); }
  void clearScreen() { system("cls"); }

#else
  #include <termios.h>
  #include <unistd.h>
  #include <sys/ioctl.h>

  static termios orig_termios;
  static bool termios_initialized = false;

  void initInput() {
      if (termios_initialized) return;
      tcgetattr(STDIN_FILENO, &orig_termios);
      termios raw = orig_termios;
      raw.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &raw);
      termios_initialized = true;
  }
  void resetInput() {
      if (!termios_initialized) return;
      tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
      termios_initialized = false;
  }
  bool kbhitCheck() {
      int bytesWaiting = 0;
      ioctl(STDIN_FILENO, FIONREAD, &bytesWaiting);
      return bytesWaiting > 0;
  }
  char getChar() {
      char c = 0;
      read(STDIN_FILENO, &c, 1);
      return c;
  }
  void setCursorPos(int X, int Y) {
      // ANSI: row(Y+1);col(X+1)
      std::printf("\033[%d;%dH", Y + 1, X + 1);
  }
  void HideCursor() { std::cout << "\033[?25l"; }
  void ShowCursor() { std::cout << "\033[?25h"; }
  void clearScreen() { system("clear"); }
#endif
// ===================================================

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

// Shared atomics
static std::atomic<bool> playAgain(true);
static std::atomic<int>  bestScore(0);

// ---------------------------------------------------
// Board
// ---------------------------------------------------
class Board {
    int width, height;
public:
    Board(int w, int h) : width(w), height(h) {}
    int getWidth()  const { return width; }
    int getHeight() const { return height; }

    void drawBorderLine() const {
        for (int i = 0; i < width + 2; ++i) std::cout << "💎";
        std::cout << "\n";
    }
};

// ---------------------------------------------------
// Food / Bomb
// ---------------------------------------------------
class Food {
protected:
    int x = 0, y = 0;
public:
    Food() = default;
    virtual void generate(int width, int height) {
        x = std::rand() % width;
        y = std::rand() % height;
    }
    int getX() const { return x; }
    int getY() const { return y; }
    virtual void draw() const { std::cout << "🍎"; }
};

class Bomb : public Food {
public:
    void draw() const override { std::cout << "💣"; }
};

// ---------------------------------------------------
// Snake
// ---------------------------------------------------
class Snake {
    int x, y;
    int tailX[200], tailY[200];
    int nTail;
    Direction dir;
public:
    Snake(int startX, int startY) : x(startX), y(startY), nTail(3), dir(STOP) {
        // Place 3 segments to the left of head
        for (int i = 0; i < nTail; ++i) {
            tailX[i] = x - (i + 1);
            tailY[i] = y;
        }
    }

    void setDirection(Direction d) {
        // Disallow 180° instant turns
        if ((dir == LEFT && d == RIGHT) || (dir == RIGHT && d == LEFT) ||
            (dir == UP   && d == DOWN)  || (dir == DOWN  && d == UP))
            return;
        dir = d;
    }
    Direction getDirection() const { return dir; }

    int getX() const { return x; }
    int getY() const { return y; }

    void move() {
        int prevX = tailX[0], prevY = tailY[0];
        int prev2X, prev2Y;

        tailX[0] = x; tailY[0] = y;
        for (int i = 1; i < nTail; ++i) {
            prev2X = tailX[i]; prev2Y = tailY[i];
            tailX[i] = prevX;  tailY[i] = prevY;
            prevX = prev2X;    prevY = prev2Y;
        }

        switch (dir) {
            case LEFT:  --x; break;
            case RIGHT: ++x; break;
            case UP:    --y; break;
            case DOWN:  ++y; break;
            case STOP: default: break;
        }
    }

    void grow() { if (nTail < 200) ++nTail; }

    bool checkSelfCollision() const {
        return checkTailCollision(x, y);
    }

    bool checkTailCollision(int tx, int ty) const {
        for (int i = 0; i < nTail; ++i)
            if (tailX[i] == tx && tailY[i] == ty) return true;
        return false;
    }

    bool checkPointCollision(int tx, int ty) const {
        if (x == tx && y == ty) return true;
        return checkTailCollision(tx, ty);
    }

    bool checkWallCollision(const Board& board) const {
        return (x < 0 || x >= board.getWidth() || y < 0 || y >= board.getHeight());
    }
};

// ---------------------------------------------------
// Game
// ---------------------------------------------------
class Game {
    int boardSize = 20;           // NxN
    Board board{boardSize, boardSize};

    int numSnakes = 2;
    std::vector<Snake*> snakes;
    std::vector<int> scores;
    Food*  fruit = nullptr;
    Bomb*  bomb  = nullptr;

    std::string playerName;
    int  delayTime = 120;
    bool gameRunning = false;
    bool bombHit = false;
    bool firstTime = true;

public:
    Game() {
        std::srand(static_cast<unsigned>(time(nullptr)));
        fruit = new Food();
        bomb  = new Bomb();
    }
    ~Game() { 
        for (auto s : snakes) delete s;
        delete fruit; 
        delete bomb; 
    }

    void setup() {
        bombHit = false;
        gameRunning = true;

        for (auto s : snakes) delete s;
        snakes.clear();
        scores.assign(numSnakes, 0);
        for (int i = 0; i < numSnakes; ++i) {
            snakes.push_back(new Snake(boardSize / 2, boardSize / 2 + (i * 2)));
        }

        fruit->generate(boardSize, boardSize);
        bomb->generate(boardSize, boardSize);

        HideCursor();
    }

    void showIntro() {
        clearScreen();
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
#endif
        if (firstTime) {
            std::cout << "\n💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎\n";
            std::cout << "          WELCOME TO SNAKE QUEST \n";
            std::cout << "💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎💎\n\n";
            std::cout << "👤 Enter your name: ";
            std::cin  >> playerName;
            firstTime = false;
        } else {
            std::cout << "\n Welcome back, " << playerName << "! Ready for another round?\n";
        }

        chooseLevel();
        std::cout << "\n🚀 Setting up your arena...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    }

    void chooseLevel() {
        std::cout << "\n Choose difficulty:\n";
        std::cout << "   1 Easy   (Relaxed)\n";
        std::cout << "   2 Medium (Classic)\n";
        std::cout << "   3 Hard   (Blink and Die)\n";
        std::cout << " Enter choice: ";
        int choice = 2;
        std::cin >> choice;

        if (choice == 1)      delayTime = 180;
        else if (choice == 2) delayTime = 120;
        else if (choice == 3) delayTime = 70;
        else                  delayTime = 120;
    }

    void updateScoreUI() {
        setCursorPos(0, 0);
        std::cout << "🎮 P1: " << (scores.size() > 0 ? scores[0] : 0)
                  << "  🎮 P2: " << (scores.size() > 1 ? scores[1] : 0)
                  << "    🏆 High: " << bestScore.load() << "     ";
    }

    void draw() const {
        setCursorPos(0, 1);
        board.drawBorderLine();
        for (int i = 0; i < board.getHeight(); ++i) {
            std::cout << "💎";
            for (int j = 0; j < board.getWidth(); ++j) {
                bool drawn = false;
                for (auto s : snakes) {
                    if (s->getX() == j && s->getY() == i) { std::cout << "😶"; drawn = true; break; }
                }
                if (drawn) continue;

                if (i == fruit->getY() && j == fruit->getX()) { fruit->draw(); drawn = true; }
                else if (i == bomb->getY() && j == bomb->getX()) { bomb->draw(); drawn = true; }
                if (drawn) continue;

                for (auto s : snakes) {
                    if (s->checkTailCollision(j, i)) { std::cout << "🟡"; drawn = true; break; }
                }
                if (!drawn) std::cout << "  ";
            }
            std::cout << "💎\n";
        }
        board.drawBorderLine();
    }

    void input() {
        if (kbhitCheck()) {
            char c = getChar();
            switch (c) {
                case 'w': case 'W': if (!snakes.empty()) snakes[0]->setDirection(UP);    break;
                case 's': case 'S': if (!snakes.empty()) snakes[0]->setDirection(DOWN);  break;
                case 'a': case 'A': if (!snakes.empty()) snakes[0]->setDirection(LEFT);  break;
                case 'd': case 'D': if (!snakes.empty()) snakes[0]->setDirection(RIGHT); break;
                
                case 'i': case 'I': if (snakes.size() > 1) snakes[1]->setDirection(UP);    break;
                case 'k': case 'K': if (snakes.size() > 1) snakes[1]->setDirection(DOWN);  break;
                case 'j': case 'J': if (snakes.size() > 1) snakes[1]->setDirection(LEFT);  break;
                case 'l': case 'L': if (snakes.size() > 1) snakes[1]->setDirection(RIGHT); break;

                case 'x': case 'X': gameRunning = false;        break;
            }
        }
    }

    void logic() {
        for (size_t i = 0; i < snakes.size(); ++i) {
            auto s = snakes[i];
            s->move();

            // Prevent instant self-collision while STOP
            if (s->getDirection() != STOP && s->checkSelfCollision())
                gameRunning = false;

            if (s->checkWallCollision(board))
                gameRunning = false;

            // Collision with other snakes
            for (size_t j = 0; j < snakes.size(); ++j) {
                if (i != j && snakes[j]->checkPointCollision(s->getX(), s->getY())) {
                    gameRunning = false;
                }
            }

            // Eat fruit
            if (s->getX() == fruit->getX() && s->getY() == fruit->getY()) {
                scores[i] += 10;
                if (scores[i] > bestScore) bestScore = scores[i];
                s->grow();
                fruit->generate(boardSize, boardSize);
                bomb->generate(boardSize, boardSize);
            }

            // Hit bomb
            if (s->getX() == bomb->getX() && s->getY() == bomb->getY()) {
                bombHit = true;
                gameRunning = false;
            }
        }
    }

    void gameOverMessage() {
        clearScreen();
        if (bombHit) {
            std::cout << "\n💣💥  Uh oh! You just hugged a bomb, " << playerName << "! 😅\n";
            std::cout << "Next time, maybe avoid the explosive stuff, okay? 😂\n\n";
        } else {
            std::cout << "\n💀 Oops! " << playerName << ", your snake crashed. \n";
            std::cout << "That’s what we call a fatal bite! 😆\n\n";
        }
        std::cout << "🎯 Final Scores - P1: " << (scores.size() > 0 ? scores[0] : 0)
                  << "  P2: " << (scores.size() > 1 ? scores[1] : 0) << "\n";
        std::cout << "🏆 High Score: " << bestScore.load() << "\n\n";
    }

    void play() {
        setup();
        while (gameRunning) {
            updateScoreUI();
            draw();
            input();
            logic();
            std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
        }
        for (auto s : snakes) delete s;
        snakes.clear();
        gameOverMessage();
    }
};

int main() {
#ifndef _WIN32
    initInput();
#endif

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    Game game;
    std::string choice;
    while (playAgain) {
        game.showIntro();
        game.play();

        std::cout << "🔁 Play again? (y/n): ";
        std::cin >> choice;
        if (choice != "y" && choice != "Y") {
            playAgain = false;
            std::cout << "\n✨ Thanks for playing Snake Quest! 💎\n";
        }
    }

#ifndef _WIN32
    resetInput();
#endif
    ShowCursor();
    return 0;
}
