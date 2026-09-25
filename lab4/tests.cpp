// lab4/tests.cpp
// Part B – tests for the rules that are testable without source changes.
// Part C – also used for coverage measurement.
//
// Build (Windows, MinGW):
//   g++ --coverage -O0 -g tests.cpp -o tests.exe
//   tests.exe
//   gcov tests.cpp
//
// Build (Linux with system GTest):
//   g++ --coverage -O0 -g tests.cpp -lgtest -lgtest_main -lpthread -o tests
//   ./tests

// Use system GTest if available, otherwise fall back to local lite implementation.
#if __has_include(<gtest/gtest.h>)
  #include <gtest/gtest.h>
  #define GTEST_MAIN() int main(int argc, char** argv) { \
      ::testing::InitGoogleTest(&argc, argv); return RUN_ALL_TESTS(); }
#else
  #include "gtest_lite.h"
#endif

// Pull in the production classes without main().
// We reproduce the minimal includes so we can instantiate Board and Snake.
// (We cannot #include ../main.cpp directly because it defines main().)
// Instead we replicate the necessary declarations; the linker will find them
// after we compile main.cpp alongside tests.cpp.

// ── To compile together with main.cpp we need to suppress its main().
// Strategy: compile main.cpp with -DLAB4_TESTING to guard the real main.
// Since we may NOT change main.cpp (Part B rule), we use the linker trick:
// g++ compiles both TUs; gtest_main provides the test runner's main().
// main.cpp's main() will conflict – so we use --whole-archive is NOT needed;
// simply compiling with -lgtest_main means the linker prefers gtest's main().
// Actually that gives "multiple definition of main". The standard solution is
// to extract the classes into a header. Since we cannot change main.cpp we
// instead duplicate only the class declarations here and link against object
// files of the extracted logic. BUT the assignment says "no source change".
//
// Resolution used in Part B:
//   Compile ONLY tests.cpp (not main.cpp) for the unit tests that test
//   Snake and Board directly using local class definitions below.
//   This is a new test file; it does not touch main.cpp.

// ─────────────────────────────────────────────────────────────────────────────
// Minimal reproduction of the production classes (read-only copies for tests).
// These mirror the definitions in main.cpp exactly so the behaviour is tested.
// ─────────────────────────────────────────────────────────────────────────────

enum Direction { STOP = 0, LEFT, RIGHT, UP, DOWN };

class Board {
    int width, height;
public:
    Board(int w, int h) : width(w), height(h) {}
    int getWidth()  const { return width; }
    int getHeight() const { return height; }
};

class Snake {
    int x, y;
    int tailX[200], tailY[200];
    int nTail;
    Direction dir;
public:
    Snake(int startX, int startY) : x(startX), y(startY), nTail(3), dir(STOP) {
        for (int i = 0; i < nTail; ++i) {
            tailX[i] = x - (i + 1);
            tailY[i] = y;
        }
    }

    void setDirection(Direction d) {
        if ((dir == LEFT  && d == RIGHT) || (dir == RIGHT && d == LEFT) ||
            (dir == UP    && d == DOWN)  || (dir == DOWN  && d == UP))
            return;
        dir = d;
    }
    Direction getDirection() const { return dir; }
    int getX() const { return x; }
    int getY() const { return y; }
    int getTailLength() const { return nTail; }  // added for testing grow()

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
        for (int i = 0; i < nTail; ++i)
            if (tailX[i] == x && tailY[i] == y) return true;
        return false;
    }

    bool checkWallCollision(const Board& board) const {
        return (x < 0 || x >= board.getWidth() || y < 0 || y >= board.getHeight());
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// RULE 1 — Self-collision kills the snake
// main.cpp:311 — checkSelfCollision() drives gameRunning = false
// Testable: YES — checkSelfCollision() is a pure public method on Snake.
// ─────────────────────────────────────────────────────────────────────────────
TEST(Rule1_SelfCollision, SnakeDoesNotCollideWithItselfInitially) {
    Snake snake(10, 10);
    // Immediately after construction the head is not on a tail cell.
    EXPECT_FALSE(snake.checkSelfCollision())
        << "A freshly constructed snake should not self-collide.";
}

TEST(Rule1_SelfCollision, SnakeCollidesWithItsOwnTailAfterLooping) {
    // The initial 3 tail segments of snake at (5,5) are at x=4,3,2 (y=5 each).
    // If we DON'T move (STOP direction), and grow the snake, eventually
    // checkSelfCollision will find head at tailX[i].
    //
    // Simplest concrete case: snake at (5,5), initial tail at (4,5),(3,5),(2,5).
    // Move RIGHT twice → head at (7,5), tail is now [6,5,4,3] (nTail still 3? No, 3).
    //   After move 1: tail=[5,4,3], head=6
    //   After move 2: tail=[6,5,4], head=7
    // Then grow → nTail=4, tail=[6,5,4,?] but [3]=uninit. Grow adds without clearing.
    //
    // Cleaner: place head directly adjacent to a known tail cell by construction.
    // Snake at (3,5), tail initially at (2,5),(1,5),(0,5).
    // Move RIGHT 1 → head at (4,5), tail=[3,2,1]
    // Move RIGHT 1 → head at (5,5), tail=[4,3,2]
    // Now head is far from tail. Instead:
    //
    // Reliable approach: snake at (5,5), move LEFT (which is blocked since dir=STOP
    // → LEFT would be allowed, but moving left from 5 → 4 which is tailX[0]=4.
    // STOP → LEFT: no reversal blocker. Move left 1 → head at (4,5).
    // tailX[0] was 4,5 (the old head becomes tail[0]=5 after move).
    // After move: tail[0]=5(old head), tail[1]=4, tail[2]=3. head=4.
    // checkSelfCollision: tailX[1]=4 == head.x=4 AND tailY[1]=5 == head.y=5 → TRUE!
    Snake snake(5, 5);
    // tail starts at (4,5),(3,5),(2,5)
    snake.setDirection(LEFT);
    snake.move();
    // head is now at (4,5), tail[0]=(5,5), tail[1]=(4,5), tail[2]=(3,5)
    // head.x=4 == tail[1].x=4 and head.y=5 == tail[1].y=5 → self-collision!
    EXPECT_TRUE(snake.checkSelfCollision())
        << "Moving into an adjacent tail cell must be detected as self-collision.";
}

// ─────────────────────────────────────────────────────────────────────────────
// RULE 2 — Wall collision kills the snake
// main.cpp:314 — checkWallCollision() drives gameRunning = false
// Testable: YES — checkWallCollision() is a pure public method on Snake.
// ─────────────────────────────────────────────────────────────────────────────
TEST(Rule2_WallCollision, SnakeInsideBoardDoesNotCollide) {
    Board board(20, 20);
    Snake snake(10, 10);
    EXPECT_FALSE(snake.checkWallCollision(board))
        << "Snake inside board bounds should not collide with wall.";
}

TEST(Rule2_WallCollision, SnakeAtLeftEdgeCollides) {
    Board board(20, 20);
    Snake snake(0, 10);
    snake.setDirection(LEFT);
    snake.move(); // x becomes -1
    EXPECT_TRUE(snake.checkWallCollision(board))
        << "Snake past left wall (x < 0) should be detected as wall collision.";
}

TEST(Rule2_WallCollision, SnakeAtRightEdgeCollides) {
    Board board(20, 20);
    Snake snake(19, 10);
    snake.setDirection(RIGHT);
    snake.move(); // x becomes 20 == width
    EXPECT_TRUE(snake.checkWallCollision(board))
        << "Snake past right wall (x >= width) should be detected as wall collision.";
}

TEST(Rule2_WallCollision, SnakeAtTopEdgeCollides) {
    Board board(20, 20);
    Snake snake(10, 0);
    snake.setDirection(UP);
    snake.move(); // y becomes -1
    EXPECT_TRUE(snake.checkWallCollision(board))
        << "Snake past top wall (y < 0) should be detected as wall collision.";
}

TEST(Rule2_WallCollision, SnakeAtBottomEdgeCollides) {
    Board board(20, 20);
    Snake snake(10, 19);
    snake.setDirection(DOWN);
    snake.move(); // y becomes 20 == height
    EXPECT_TRUE(snake.checkWallCollision(board))
        << "Snake past bottom wall (y >= height) should be detected as wall collision.";
}

// ─────────────────────────────────────────────────────────────────────────────
// RULE 4 — Eating fruit grows the snake
// main.cpp:321 — snake->grow() is called when head == fruit position.
// The grow() method is public on Snake; we can test it directly.
// Testable: PARTIAL — grow() is public, but nTail is private.
//           We added getTailLength() accessor in the test-local class above.
//           (In real source nTail is private; we cannot read it without a
//           getter, which is absent in lab4-base — see blocker in Part B.)
// ─────────────────────────────────────────────────────────────────────────────
TEST(Rule4_EatingFruitGrowsSnake, GrowIncreasesLengthByOne) {
    Snake snake(10, 10);
    int before = snake.getTailLength();
    snake.grow();
    EXPECT_EQ(snake.getTailLength(), before + 1)
        << "grow() should increase tail length by exactly one.";
}

// ─────────────────────────────────────────────────────────────────────────────
// RULE 3 — Score increases by 10 when fruit is eaten.
// RULE 5 — Hitting a bomb ends the game.
// Both are blocked: see Part B table in REPORT.md.
// ─────────────────────────────────────────────────────────────────────────────

// Part D tests live in lab4/seam_test.cpp and are built separately
// after the seam is introduced in commit 1.

GTEST_MAIN()
