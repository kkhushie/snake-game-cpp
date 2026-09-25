// lab4/seam_test.cpp
// Part D – the test written AFTER the seam.
// This file tests Rule 3: eating fruit increments the score by exactly 10.
//
// Build (Windows, MinGW — uses gtest_lite.h):
//   g++ --coverage -O0 -g seam_test.cpp -o seam_test.exe
//   seam_test.exe
//
// Build (Linux with system GTest):
//   g++ --coverage -O0 -g seam_test.cpp -lgtest -lgtest_main -lpthread -o seam_test

#if __has_include(<gtest/gtest.h>)
  #include <gtest/gtest.h>
  #define GTEST_MAIN() int main(int argc, char** argv) { \
      ::testing::InitGoogleTest(&argc, argv); return RUN_ALL_TESTS(); }
#else
  #include "gtest_lite.h"
#endif
#include <string>

// ─────────────────────────────────────────────────────────────────────────────
// Seam interface (mirrors the one added to main_seam.cpp in commit 1)
// ─────────────────────────────────────────────────────────────────────────────
struct ScorePolicy {
    virtual void onFruitEaten(int& score, int& bestScore) = 0;
    virtual ~ScorePolicy() = default;
};

// Production policy — unchanged behaviour (commit 1 extracts this)
struct DefaultScorePolicy : ScorePolicy {
    void onFruitEaten(int& score, int& bestScore) override {
        score += 10;
        if (score > bestScore) bestScore = score;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Part E — Test double: SPY
// The collaborator (ScorePolicy) is *told* to do something (update score).
// Because onFruitEaten returns void, the only way to assert its effect is to
// observe either side-effects on score or that the call happened.
// A Spy records the call count; an assert on the score value also works here.
// We use a Spy because the production interface returns void.
// ─────────────────────────────────────────────────────────────────────────────
struct ScoreSpy : ScorePolicy {
    int callCount = 0;
    int lastScoreSeen = -1;

    void onFruitEaten(int& score, int& bestScore) override {
        ++callCount;
        lastScoreSeen = score;
        score += 10;                           // real behaviour preserved
        if (score > bestScore) bestScore = score;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Minimal seam-aware game logic (extracted from main_seam.cpp)
// ─────────────────────────────────────────────────────────────────────────────
enum Direction2 { STOP2=0, LEFT2, RIGHT2, UP2, DOWN2 };

struct Pos { int x, y; };

// Minimal snake with just position and eat detection
struct MinSnake {
    Pos head;
    MinSnake(int x, int y) : head{x,y} {}
    bool eats(Pos food) const { return head.x == food.x && head.y == food.y; }
    void moveTo(Pos p) { head = p; }
};

// Seam-aware logic function (mirrors what commit 1 does in Game::logic)
void runLogic(MinSnake& snake, Pos& fruit, int& score, int& bestScore,
              ScorePolicy& policy) {
    if (snake.eats(fruit)) {
        policy.onFruitEaten(score, bestScore);
        // In the real game, fruit.generate() is called here; we skip it.
        fruit = {-1, -1};  // mark eaten
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RULE 3 — Eating fruit adds exactly 10 to the score
// ─────────────────────────────────────────────────────────────────────────────
TEST(Rule3_ScoreIncrement, EatingFruitAddsTenToScore) {
    MinSnake snake(5, 5);
    Pos fruit{5, 5};   // fruit at same position as head → immediate eat
    int score = 0, bestScore = 0;
    DefaultScorePolicy policy;

    runLogic(snake, fruit, score, bestScore, policy);

    EXPECT_EQ(score, 10)
        << "Score must increase by exactly 10 when a fruit is eaten.";
}

TEST(Rule3_ScoreIncrement, ScoreAccumulatesCorrectlyOverMultipleEats) {
    MinSnake snake(5, 5);
    int score = 0, bestScore = 0;
    DefaultScorePolicy policy;

    for (int i = 0; i < 3; ++i) {
        Pos fruit{5, 5};
        runLogic(snake, fruit, score, bestScore, policy);
    }

    EXPECT_EQ(score, 30)
        << "After three fruits, score must be exactly 30.";
}

TEST(Rule3_ScoreIncrement, SpyRecordsThatPolicyWasCalledOnEat) {
    MinSnake snake(5, 5);
    Pos fruit{5, 5};
    int score = 0, bestScore = 0;
    ScoreSpy spy;

    runLogic(snake, fruit, score, bestScore, spy);

    EXPECT_EQ(spy.callCount, 1)
        << "onFruitEaten must be called exactly once per fruit eaten.";
    EXPECT_EQ(score, 10)
        << "Score must be 10 after spy forwards the call.";
}

TEST(Rule3_ScoreIncrement, NoScoreChangeWhenSnakeMissesFood) {
    MinSnake snake(3, 3);
    Pos fruit{7, 7};  // different position
    int score = 0, bestScore = 0;
    ScoreSpy spy;

    runLogic(snake, fruit, score, bestScore, spy);

    EXPECT_EQ(spy.callCount, 0)
        << "onFruitEaten must NOT be called when head doesn't reach the fruit.";
    EXPECT_EQ(score, 0)
        << "Score must not change if the snake did not eat the fruit.";
}

GTEST_MAIN()
