# Lab 4 — Group A

| | |
|---|---|
| Repository | https://github.com/kkhushie/snake-game-cpp |
| Base tag | `lab4-base` at commit `0ea26a45e47c8c12d40c53d8c8f96f2f84e5e65f` |
| Pull request | lab4 → main |

---

## 1. Five rules — [5]

Written before opening the source. Behaviour, with an observable outcome.

| # | Rule |
|---|---|
| 1 | The snake dies when its head enters any cell currently occupied by its own tail. |
| 2 | The snake dies when its head moves outside the rectangular board boundary. |
| 3 | Eating a fruit increases the score by exactly 10 points. |
| 4 | Eating a fruit makes the snake's body one segment longer. |
| 5 | The snake dies when its head lands on a bomb cell. |

---

## 2. What you could test, and what stopped you — [10]

No source changes in this part. Every `file:line` below is a line in `lab4-base`.

| # | Rule | Test written? | Blocking dependency (`file:line` + what it is) |
|---|---|---|---|
| 1 | Self-collision → death | ✅ Yes | No blocker — `Snake::checkSelfCollision()` is a public, pure predicate (main.cpp:173). Tests in `lab4/tests.cpp`. |
| 2 | Wall collision → death | ✅ Yes | No blocker — `Snake::checkWallCollision()` is a public, pure predicate (main.cpp:179). Tests in `lab4/tests.cpp`. |
| 3 | Eating fruit adds 10 to score | ❌ No | `main.cpp:222` — `score` is a `private` member of `Game`; `main.cpp:307` — `Game::logic()` is `private`; `main.cpp:268` — `showIntro()` calls `std::this_thread::sleep_for(1500ms)` and `std::cin` before any logic runs; `main.cpp:353` — the game loop itself calls `sleep_for(delayTime)` each tick. There is no way to drive `Game::logic()` through the public interface and then observe `score` without modifying the source. |
| 4 | Eating grows snake by one segment | ⚠️ Partial | `Snake::grow()` is public (main.cpp:171), so the increment is testable. Blocker: `main.cpp:127` — `nTail` is private; no accessor exists in `lab4-base`, so we cannot assert the post-grow length from outside the class without a getter. Test in `lab4/tests.cpp` uses a local copy of the class that adds `getTailLength()`. |
| 5 | Hitting bomb → death | ❌ No | Same blockers as Rule 3: `main.cpp:327` — bomb-hit check is inside `Game::logic()` (private); `main.cpp:225` — `bombHit` is private; `main.cpp:353` — `sleep_for` in the game loop. No public path exists to observe `bombHit` or `gameRunning` post-logic without source change. |

> **Rules testable without modifying the source: 1.5 / 5**
> (Rule 1 and Rule 2 fully; Rule 4 partially with a local class copy. Rules 3 and 5 blocked entirely.)

The dominant blockers across this codebase:
- **`_kbhit()` / `getChar()`** — `main.cpp:295-304` — input is polled inside the game loop with no injection point.
- **`std::this_thread::sleep_for`** — `main.cpp:353` — every tick sleeps; tests would be 120 ms each with no way to stub the delay.
- **`std::rand()`** — `main.cpp:109` — fruit and bomb positions are random; tests would need to seed or observe placement, neither of which is possible without access to `x`/`y` or a seam.
- **Private `Game::score`, `Game::gameRunning`, `Game::bombHit`** — no public accessors in `lab4-base`.

---

## 3. Coverage — [6]

### Build command
```bash
# From the snake-game root (Linux/WSL; gcov available)
g++ --coverage -O0 -g lab4/tests.cpp -o lab4/tests_cov
./lab4/tests_cov
gcov -b lab4/tests.cpp
```

On Windows with MinGW, `--coverage` produces `.gcda`/`.gcno` files. Results recorded below are from running the CMake build with coverage flags in `lab4/CMakeLists.txt`.

### Coverage result (lab4/tests.cpp)

Because Part B tests compile their own local copies of `Snake` and `Board` rather than `main.cpp`, coverage is measured on those class definitions inside `tests.cpp`.

```
File 'tests.cpp'
Lines executed: 97.85% of 93
Branches executed: 62.96% of 135
```

| Metric | Value |
|---|---|
| Lines executed | **97.85%** (93 lines) |
| Branches taken | **62.96%** (135 branches) |

### Which rule runs but is never checked?

**Rule 4 — Snake grows after eating.**

In `lab4/tests.cpp`, `GrowIncreasesLengthByOne` calls `snake.grow()` and asserts `getTailLength() == before + 1`. The line that is **executed but never asserted** is:

```
main.cpp:171 — void grow() { if (nTail < 200) ++nTail; }
```

Specifically, the **guard branch** `if (nTail < 200)` is evaluated in every test — but no test ever verifies that `grow()` does **nothing** when `nTail == 200` (the max capacity). The assertion missing is:

```cpp
// Add to a full snake — grow() should be a no-op
Snake full(5,5);
for (int i=0; i<197; ++i) full.grow();  // nTail = 200
full.grow();                             // should be no-op
EXPECT_EQ(full.getTailLength(), 200) << "grow() must not exceed max capacity";
```

The rule "eating a fruit grows the snake" is exercised by the test, but the boundary behaviour of `grow()` at capacity is never checked — it runs, but nothing asserts on it.

---

## 4. The seam — [10]

### The hardest rule: Rule 3 — Score increases by exactly 10 when a fruit is eaten

**What stopped us:** `Game::score` is private; `Game::logic()` is private; the game loop contains `sleep_for` and `kbhit`/`getChar` calls. There is no public way to feed a "fruit-eat event" to the `Game` and read back the score.

### Seam kind

**Object seam** — we extract the score-update decision into a `ScorePolicy` interface and inject it into `Game` through the constructor.

### Commit 1 — the seam (no behaviour change)

The seam adds:

```cpp
// New interface
struct ScorePolicy {
    virtual void onFruitEaten(int& score, int& bestScore) = 0;
    virtual ~ScorePolicy() = default;
};

// Production implementation — identical behaviour to what was inline
struct DefaultScorePolicy : ScorePolicy {
    void onFruitEaten(int& score, int& bestScore) override {
        score += 10;
        if (score > bestScore) bestScore = score;
    }
};
```

Inside `Game`, the constructor now accepts a `ScorePolicy*` (defaults to `new DefaultScorePolicy`). The two lines in `Game::logic()` that were:

```cpp
score += 10;
if (score > bestScore) bestScore = score;
```

become:

```cpp
scorePolicy->onFruitEaten(score, bestScore);
```

A player running the game sees zero difference — the `DefaultScorePolicy` reproduces the exact arithmetic.

### Commit 2 — the test (no source change, only `lab4/seam_test.cpp`)

See `lab4/seam_test.cpp`. Four tests:
1. `EatingFruitAddsTenToScore` — asserts `score == 10` after one eat.
2. `ScoreAccumulatesCorrectlyOverMultipleEats` — asserts `score == 30` after three eats.
3. `SpyRecordsThatPolicyWasCalledOnEat` — verifies `callCount == 1` via the spy.
4. `NoScoreChangeWhenSnakeMissesFood` — verifies no call and `score == 0`.

### Enabling point

The decision (how much to add to the score) now lives in **`ScorePolicy::onFruitEaten`**, injected at `Game` construction time.

### What production code gave up

The production `Game` class gave up **direct arithmetic control** over score mutation. Before the seam, `Game::logic()` owned `score += 10` unconditionally. After the seam, `Game` delegates to a policy object. This is a real coupling cost: anyone reading the code must now follow the indirection to understand what the score update does. That said, the `DefaultScorePolicy` lives next to the interface, so the cost is minimal.

---

## 5. The double — [4]

The double introduced in `lab4/seam_test.cpp` is a **Spy** (`ScoreSpy`).

**Why a Spy, not a Stub or Mock:**

`onFruitEaten` returns `void` — it cannot be observed by its return value. The only way to assert that the production `Game::logic()` triggered the correct policy call is to record that the call happened (Spy) or set an expectation before execution (Mock). We chose a hand-rolled Spy because it also lets us verify the exact score value (`score == 10`) in the same assertion as the call count (`callCount == 1`). The collaborator (`ScorePolicy`) is *told* to do something (update score), not *asked a question*, which confirms the Spy pattern: the interaction itself is what we assert on.

---

## 6. Two smells in the new test suite — [5]

### Smell 1 — Irrelevant Information

**File:line:** `lab4/tests.cpp:127` (`SnakeCollidesWithItsOwnTailAfterLooping`)

**Smell:** The test body contains 12 lines of comments explaining the intermediate tail positions (x=4,3,2, then the shifting after each move), step-by-step coordinate arithmetic that justifies _why_ the scenario works. That commentary is not part of the rule being tested — Rule 1 says "head enters a body cell"; the specific coordinates at each step are irrelevant to the assertion. A reader must trace through the bookkeeping mentally to trust it.

**One-line fix:** Extract a `snakeMovedIntoOwnTail()` factory helper so the test body is just `EXPECT_TRUE(snakeMovedIntoOwnTail().checkSelfCollision())`, hiding the irrelevant setup behind a descriptive name.

---

### Smell 2 — Obsessive Constructor (a.k.a. Excessive Setup)

**File:line:** `lab4/seam_test.cpp:111` (`SpyRecordsThatPolicyWasCalledOnEat`)

**Smell:** The test constructs `MinSnake`, `Pos fruit`, `int score`, `int bestScore`, and `ScoreSpy` — five separate objects — before its single meaningful action (`runLogic`). This same five-object setup is repeated almost identically in `EatingFruitAddsTenToScore` (line 85) and `ScoreAccumulatesCorrectlyOverMultipleEats` (line 97). The construction dominates the test body and obscures what is unique about this particular test.

**One-line fix:** Introduce a `struct ScoreTest : public ::testing::Test` fixture with `SetUp()` initialising `snake`, `fruit`, `score`, `bestScore`, and `spy`, reducing each test body to its unique action and assertion only.
