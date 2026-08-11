#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <string>
#include <atomic>
#include <vector>
#include <sstream>

// ===================================================
// Platform specifics
// ===================================================
#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>

  static HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  static DWORD dwOriginalOutMode = 0;

  void initTerminal() {
      GetConsoleMode(hOut, &dwOriginalOutMode);
      DWORD dwOutMode = dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
      SetConsoleMode(hOut, dwOutMode);
      SetConsoleOutputCP(CP_UTF8);
  }
  void restoreTerminal() {
      SetConsoleMode(hOut, dwOriginalOutMode);
  }

  void setCursorPos(int X, int Y) {
      std::cout << "\033[" << (Y + 1) << ";" << (X + 1) << "H";
  }
  void HideCursor() { std::cout << "\033[?25l"; }
  void ShowCursor() { std::cout << "\033[?25h"; }
  bool kbhitCheck() { return _kbhit(); }
  char getChar() { return _getch(); }
  void clearScreen() { std::cout << "\033[2J\033[H"; }

  void initInput() { }
  void resetInput() { }

#else
  #include <termios.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
  #include <fcntl.h>

  static termios orig_termios;
  static bool termios_initialized = false;

  void initTerminal() {}
  void restoreTerminal() {}

  void initInput() {
      if (termios_initialized) return;
      tcgetattr(STDIN_FILENO, &orig_termios);
      termios raw = orig_termios;
      raw.c_lflag &= ~(ICANON | ECHO);
      raw.c_cc[VMIN] = 0;
      raw.c_cc[VTIME] = 0;
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
      if (read(STDIN_FILENO, &c, 1) <= 0) return 0;
      return c;
  }
  void setCursorPos(int X, int Y) {
      std::cout << "\033[" << (Y + 1) << ";" << (X + 1) << "H";
  }
  void HideCursor() { std::cout << "\033[?25l"; }
  void ShowCursor() { std::cout << "\033[?25h"; }
  void clearScreen() { std::cout << "\033[2J\033[H"; }
#endif
// ===================================================

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

static std::atomic<bool> playAgain(true);
static std::atomic<int>  bestScore(0);

// ---------------------------------------------------
// Food / Bomb
// ---------------------------------------------------
class Food {
protected:
    int x = 0, y = 0;
public:
    Food() = default;
    virtual void generate(int width, int height) {
        x = 1 + std::rand() % (width - 2);
        y = 1 + std::rand() % (height - 2);
    }
    int getX() const { return x; }
    int getY() const { return y; }
    virtual std::string getIcon() const { return "\033[91m●\033[0m"; } // Red
};

class Bomb : public Food {
public:
    std::string getIcon() const override { return "\033[93m×\033[0m"; } // Yellow
};

// ---------------------------------------------------
// Snake
// ---------------------------------------------------
class Snake {
public:
    int x, y;
    int tailX[200], tailY[200];
    int nTail;
    Direction dir;
    std::string color;
    
    Snake(int startX, int startY, std::string col) : x(startX), y(startY), nTail(3), dir(STOP), color(col) {
        for (int i = 0; i < nTail; ++i) {
            tailX[i] = x - (i + 1);
            tailY[i] = y;
        }
    }

    void setDirection(Direction d) {
        if ((dir == LEFT && d == RIGHT) || (dir == RIGHT && d == LEFT) ||
            (dir == UP   && d == DOWN)  || (dir == DOWN  && d == UP))
            return;
        if (dir == STOP && d == LEFT) return;
        dir = d;
    }
    
    Direction getDirection() const { return dir; }
    int getX() const { return x; }
    int getY() const { return y; }

    void move() {
        if (dir == STOP) return;

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
        for (int i = 0; i < nTail; ++i)
            if (tailX[i] == x && tailY[i] == y) return true;
        return false;
    }

    bool checkWallCollision(int width, int height) const {
        return (x <= 0 || x >= width - 1 || y <= 0 || y >= height - 1);
    }

    bool isBodyAt(int bx, int by) const {
        for (int k = 0; k < nTail; ++k) {
            if (tailX[k] == bx && tailY[k] == by) return true;
        }
        return false;
    }

    bool checkCollisionWith(const Snake& other) const {
        if (x == other.x && y == other.y) return true;
        if (other.isBodyAt(x, y)) return true;
        return false;
    }
};

// ---------------------------------------------------
// Game
// ---------------------------------------------------
class Game {
    int boardWidth = 50;
    int boardHeight = 25;

    Snake* snake1 = nullptr;
    Snake* snake2 = nullptr;
    Food*  fruit = nullptr;
    Bomb*  bomb  = nullptr;

    std::string prevBuffer[30][60];
    bool forceRedraw = true;

    std::string playerName;
    int  score1 = 0;
    int  score2 = 0;
    int  delayTime = 100;
    bool gameRunning = false;
    bool firstTime = true;
    int loser = 0;

public:
    Game() {
        std::srand(static_cast<unsigned>(time(nullptr)));
        fruit = new Food();
        bomb  = new Bomb();
    }
    ~Game() { delete fruit; delete bomb; }

    void setup() {
        score1 = 0;
        score2 = 0;
        loser = 0;
        gameRunning = true;
        forceRedraw = true;

        delete snake1;
        delete snake2;
        snake1 = new Snake(boardWidth / 3, boardHeight / 2, "\033[92m█\033[0m"); // Green
        snake2 = new Snake(2 * boardWidth / 3, boardHeight / 2, "\033[94m█\033[0m"); // Blue

        fruit->generate(boardWidth, boardHeight);
        bomb->generate(boardWidth, boardHeight);
        
        clearScreen();
    }

    void showIntro() {
        clearScreen();
        if (firstTime) {
            std::cout << "\n============================================\n";
            std::cout << "          WELCOME TO SNAKE QUEST \n";
            std::cout << "============================================\n\n";
            std::cout << "Enter your name: ";
            std::cin  >> playerName;
            std::cin.ignore(10000, '\n');
            firstTime = false;
        } else {
            std::cout << "\n Welcome back, " << playerName << "! Ready for another round?\n";
        }

        std::cout << "\n Choose difficulty:\n";
        std::cout << "   1 Easy   (Relaxed)\n";
        std::cout << "   2 Medium (Classic)\n";
        std::cout << "   3 Hard   (Blink and Die)\n";
        std::cout << " Enter choice: ";
        
        std::string choiceStr;
        std::getline(std::cin, choiceStr);
        int choice = 2;
        try { choice = std::stoi(choiceStr); } catch(...) {}

        if (choice == 1)      delayTime = 120;
        else if (choice == 2) delayTime = 80;
        else if (choice == 3) delayTime = 50;
        else                  delayTime = 80;

        std::cout << "\n🚀 Setting up your arena...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    void drawFrame() {
        std::string nextBuffer[30][60];
        
        for(int y = 0; y < boardHeight; ++y) {
            for(int x = 0; x < boardWidth; ++x) {
                if (y == 0 || y == boardHeight - 1 || x == 0 || x == boardWidth - 1) {
                    nextBuffer[y][x] = "\033[96m█\033[0m"; // Cyan border
                } else {
                    nextBuffer[y][x] = " ";
                }
            }
        }
        
        // Draw items
        nextBuffer[fruit->getY()][fruit->getX()] = fruit->getIcon();
        nextBuffer[bomb->getY()][bomb->getX()] = bomb->getIcon();
        
        // Draw snake 2
        for(int i=0; i<snake2->nTail; ++i) {
            if (snake2->tailY[i] > 0 && snake2->tailY[i] < boardHeight - 1 && snake2->tailX[i] > 0 && snake2->tailX[i] < boardWidth - 1)
                nextBuffer[snake2->tailY[i]][snake2->tailX[i]] = snake2->color;
        }
        if (snake2->getY() > 0 && snake2->getY() < boardHeight - 1 && snake2->getX() > 0 && snake2->getX() < boardWidth - 1)
            nextBuffer[snake2->getY()][snake2->getX()] = snake2->color;
            
        // Draw snake 1
        for(int i=0; i<snake1->nTail; ++i) {
            if (snake1->tailY[i] > 0 && snake1->tailY[i] < boardHeight - 1 && snake1->tailX[i] > 0 && snake1->tailX[i] < boardWidth - 1)
                nextBuffer[snake1->tailY[i]][snake1->tailX[i]] = snake1->color;
        }
        if (snake1->getY() > 0 && snake1->getY() < boardHeight - 1 && snake1->getX() > 0 && snake1->getX() < boardWidth - 1)
            nextBuffer[snake1->getY()][snake1->getX()] = snake1->color;

        // Render differences
        std::stringstream ss;
        for(int y = 0; y < boardHeight; ++y) {
            for(int x = 0; x < boardWidth; ++x) {
                if (forceRedraw || nextBuffer[y][x] != prevBuffer[y][x]) {
                    ss << "\033[" << (y + 2) << ";" << (x + 1) << "H" << nextBuffer[y][x];
                    prevBuffer[y][x] = nextBuffer[y][x];
                }
            }
        }
        std::cout << ss.str() << std::flush;
        forceRedraw = false;
    }

    void updateScoreUI() {
        setCursorPos(0, 0);
        std::cout << "\033[92mP1 Score: " << score1 
                  << "\033[0m     \033[94mP2 Score: " << score2
                  << "\033[0m     \033[93mHigh Score: " << bestScore.load() 
                  << "\033[0m                   "; // Pad with spaces
    }

    void input() {
        while (kbhitCheck()) {
            char c = getChar();
            if (c <= 0) continue;
            
            // Handle Arrow Keys for Player 1
            if (c == '\033') {
                char bracket = getChar();
                if (bracket == '[') {
                    char arr = getChar();
                    if (arr == 'A') snake1->setDirection(UP);
                    else if (arr == 'B') snake1->setDirection(DOWN);
                    else if (arr == 'C') snake1->setDirection(RIGHT);
                    else if (arr == 'D') snake1->setDirection(LEFT);
                }
                continue;
            } else if (c == -32 || c == 224) { // Windows arrows
                char arr = getChar();
                if (arr == 72) snake1->setDirection(UP);
                else if (arr == 80) snake1->setDirection(DOWN);
                else if (arr == 77) snake1->setDirection(RIGHT);
                else if (arr == 75) snake1->setDirection(LEFT);
                continue;
            }

            switch (c) {
                // Player 2 controls (WASD)
                case 'a': case 'A': snake2->setDirection(LEFT);  break;
                case 'd': case 'D': snake2->setDirection(RIGHT); break;
                case 'w': case 'W': snake2->setDirection(UP);    break;
                case 's': case 'S': snake2->setDirection(DOWN);  break;
                
                case 'x': case 'X': gameRunning = false;        break;
            }
        }
    }

    void logic() {
        snake1->move();
        snake2->move();

        bool s1_dies = false;
        bool s2_dies = false;

        if (snake1->getDirection() != STOP && snake1->checkSelfCollision()) s1_dies = true;
        if (snake2->getDirection() != STOP && snake2->checkSelfCollision()) s2_dies = true;

        if (snake1->checkWallCollision(boardWidth, boardHeight)) s1_dies = true;
        if (snake2->checkWallCollision(boardWidth, boardHeight)) s2_dies = true;

        if (snake1->checkCollisionWith(*snake2)) s1_dies = true;
        if (snake2->checkCollisionWith(*snake1)) s2_dies = true;
        
        if (s1_dies || s2_dies) {
            gameRunning = false;
            if (s1_dies && s2_dies) loser = 3; // both
            else if (s1_dies) loser = 1;
            else loser = 2;
        }

        // Eat fruit
        bool eaten = false;
        if (snake1->getX() == fruit->getX() && snake1->getY() == fruit->getY()) {
            score1 += 10;
            if (score1 > bestScore) bestScore = score1;
            snake1->grow();
            eaten = true;
        } else if (snake2->getX() == fruit->getX() && snake2->getY() == fruit->getY()) {
            score2 += 10;
            if (score2 > bestScore) bestScore = score2;
            snake2->grow();
            eaten = true;
        }
        if (eaten) {
            fruit->generate(boardWidth, boardHeight);
            bomb->generate(boardWidth, boardHeight);
        }

        // Hit bomb
        if (snake1->getX() == bomb->getX() && snake1->getY() == bomb->getY()) {
            gameRunning = false;
            loser = 1;
        } else if (snake2->getX() == bomb->getX() && snake2->getY() == bomb->getY()) {
            gameRunning = false;
            loser = 2;
        }
    }

    void gameOverMessage() {
        clearScreen();
        
        std::vector<std::string> lines;
        lines.push_back("========================================");
        lines.push_back("               GAME OVER                ");
        lines.push_back("========================================");
        
        if (loser == 1) {
            lines.push_back("             Player 1 Lost              ");
        } else if (loser == 2) {
            lines.push_back("             Player 2 Lost              ");
        } else {
            lines.push_back("                 Draw                   ");
        }
        
        lines.push_back("");
        lines.push_back("          P1 Score: " + std::to_string(score1));
        lines.push_back("          P2 Score: " + std::to_string(score2));
        lines.push_back("          High Score: " + std::to_string(bestScore.load()));
        lines.push_back("");
        
        int startY = boardHeight / 2 - 5;
        for (size_t i = 0; i < lines.size(); ++i) {
            setCursorPos(5, startY + i);
            std::cout << lines[i];
        }
        std::cout << "\n\n";
    }

    void play() {
        setup();
        HideCursor();
        initInput();
        
        while (gameRunning) {
            updateScoreUI();
            drawFrame();
            input();
            logic();
            std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
        }
        
        resetInput();
        ShowCursor();
        
        delete snake1; snake1 = nullptr;
        delete snake2; snake2 = nullptr;
        
        gameOverMessage();
    }
};

int main() {
    initTerminal();

    Game game;
    std::string choice;
    while (playAgain) {
        game.showIntro();
        game.play();

        std::cout << "       Play again? (y/n): ";
        std::getline(std::cin, choice);
        if (choice.empty() || (choice[0] != 'y' && choice[0] != 'Y')) {
            playAgain = false;
            std::cout << "\n       Thanks for playing!\n";
        }
    }

    std::cout << "\033[0m\n"; // clear ANSI state
    restoreTerminal();
    return 0;
}
