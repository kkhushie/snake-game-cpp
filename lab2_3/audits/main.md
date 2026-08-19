# Code Smell Audit: main

## Confirmed Smells

1. **Large Class / Divergent Change**
   - **Location:** `main.cpp:213` (`class Game`)
   - **Confidence:** C3
   - **Severity:** HIGH
   - **Reasoning:** The `Game` class has too many responsibilities: it manages core game state, handles direct input, controls timing, and prints UI text (intro, menus, game over). Any change to UI, logic, or input requires modifying this single class.

2. **Inappropriate Intimacy / Feature Envy**
   - **Location:** `main.cpp:183` (`Snake::draw`)
   - **Confidence:** C3
   - **Severity:** HIGH
   - **Reasoning:** The `Snake::draw` method acts as the global renderer for the entire game. It loops over the `Board` dimensions and directly queries `Food` and `Bomb` positions. A `Snake` should not be responsible for drawing the entire arena and other entities.

3. **Data Clumps / Primitive Obsession**
   - **Location:** `main.cpp:127` (`int tailX[200], tailY[200]`)
   - **Confidence:** C3
   - **Severity:** MEDIUM
   - **Reasoning:** The snake's tail coordinates are maintained in two parallel arrays instead of grouping X and Y together into a `Point` or `Position` struct, which makes the coordinate logic harder to manage.

4. **Global State**
   - **Location:** `main.cpp:81-82` (`static std::atomic<bool> playAgain`, `bestScore`)
   - **Confidence:** C3
   - **Severity:** MEDIUM
   - **Reasoning:** Using mutable global/static variables for core game loop control and high score tracking makes the code harder to unit test and prevents instantiating multiple independent games.

## Rejected False-Positive Candidates

1. **Smell Candidate:** Platform-specific `#ifdef` pollution (Divergent Change)
   - **Location:** `main.cpp:11-75`
   - **Confidence:** C1 (Low)
   - **Reasoning for Rejection:** While having OS-specific macros in a file is a smell in large systems, introducing abstract interface hierarchies (e.g., `IConsole`, `InputProvider`) for a simple, single-file CLI game would be Speculative Generality. The `#ifdef` blocks are appropriately grouped at the top.

2. **Smell Candidate:** Switch Statements (if-else chain)
   - **Location:** `main.cpp:280-283` (Level selection)
   - **Confidence:** C1 (Low)
   - **Reasoning for Rejection:** Using polymorphism (like a Strategy pattern) to map integer user inputs (1, 2, 3) to millisecond delay times is extreme overkill. The existing 4-line conditional is the most readable and maintainable solution here.

3. **Smell Candidate:** Complex Conditional
   - **Location:** `main.cpp:141` (`if ((dir == LEFT && d == RIGHT) || ...)`)
   - **Confidence:** C1 (Low)
   - **Reasoning for Rejection:** This boolean expression correctly prevents the snake from making an instant 180-degree turn. While slightly long, it expresses the domain rule perfectly. Abstracting this logic away provides no practical benefit and would obscure the game rules.
