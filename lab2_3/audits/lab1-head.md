# Code Smell Audit: Lab-1 Head (feat/multiplayer)

## 1. Structural / OO Abusers

### Smell 1: Duplicated Variables / Data Clumps (Parallel Logic)
- **Location**: main.cpp:368
- **Severity**: CRITICAL
- **Confidence**: C4
- **Rationale**: The Lab 1 implementation simply duplicated variables to support a second player (`snake1`, `snake2`, `score1`, `score2`, `s1_dies`, `s2_dies`). This means adding a 3rd player would require a complete rewrite of the `logic()` and `input()` functions. This is the exact design flaw our `lab2_3/design-first` refactor addressed by using a collection.

### Smell 2: Feature Envy / Broken Encapsulation
- **Location**: main.cpp:294
- **Severity**: HIGH
- **Confidence**: C4
- **Rationale**: To support multiplayer drawing, `Snake`'s internal array data (`tailX`, `tailY`, `nTail`) was made `public`. `Game::drawFrame` now directly iterates over these raw arrays instead of asking the `Snake` class to report its collision or drawing state. This breaks encapsulation and makes `Snake` a raw Data Class.

## 2. Bloaters

### Smell 3: Large Class / God Object
- **Location**: `main.cpp` (Class `Game`)
- **Severity**: HIGH
- **Confidence**: C4
- **Rationale**: The `Game` class has grown even larger. It now handles terminal input mapping for both players, individual collision checks between snakes (`checkCollisionWith`), UI rendering, buffering (`prevBuffer`), and score tracking. It violates the Single Responsibility Principle.

## 3. Couplers

### Smell 4: Global State
- **Location**: `main.cpp` (lines 90-91: `playAgain`, `bestScore`)
- **Severity**: MEDIUM
- **Confidence**: C4
- **Rationale**: These state variables are still global, carrying over the same smell from the `main` branch.

## 4. Rejected Candidates (False Positives)

1. **Primitive Obsession (Coordinates)**: While `int x, y` are used everywhere instead of a `Point` struct, introducing a `Point` struct in this single-file terminal game would add unnecessary boilerplate for little structural gain.
2. **Switch Statements (Input)**: The switch statement mapping keyboard chars to directions is standard input handling; abstracting this into a Command Pattern would over-engineer the terminal input.
3. **Magic Numbers**: The `200` array limit is a fixed terminal constraint in this context; replacing it doesn't solve the core architectural issues.
