# Ubiquitous Language

## Game Entities

| Term        | Definition                                              | Aliases to avoid      | In code                    |
| ----------- | ------------------------------------------------------- | --------------------- | -------------------------- |
| **Board**   | The grid or arena where the game takes place            | Arena                 | `Board` — `main.cpp:87`    |
| **Snake**   | The player-controlled entity that moves around the grid |                       | `Snake` — `main.cpp:125`   |
| **Food**    | An item the snake consumes to grow and increase score   | Fruit, apple          | `Food` — `main.cpp:103`, `fruit` — `main.cpp:218` ⚠ |
| **Bomb**    | A deadly obstacle that ends the game upon collision     | Explosive             | `Bomb` — `main.cpp:117`    |

## Game State & Mechanics

| Term         | Definition                                  | Aliases to avoid       | In code                        |
| ------------ | ------------------------------------------- | ---------------------- | ------------------------------ |
| **Direction**| The current heading of the snake            | Movement, heading      | `Direction` — `main.cpp:78`    |
| **Score**    | The points accumulated by eating food       | Points                 | `score` — `main.cpp:222`       |
| **Game**     | The session/loop managing state and UI      | Match, round           | `Game` — `main.cpp:213`        |

## Relationships

- A **Game** contains exactly one **Board**, one **Snake**, one **Food**, and one **Bomb**.
- A **Snake** moves in a **Direction**.
- A **Snake** colliding with itself, the **Board** boundaries, or a **Bomb** ends the **Game**.
- When a **Snake** consumes **Food**, the **Score** increases and the **Snake** grows.

## Example dialogue

> **Dev:** "When the **Snake** eats the **Food**, does the **Bomb** change location too?"
> **Domain expert:** "Yes, whenever the **Snake** consumes the **Food**, both a new **Food** and a new **Bomb** are generated on the **Board**."
> **Dev:** "And if the **Snake** hits the **Bomb**, the **Game** is over immediately?"
> **Domain expert:** "Exactly. It's an instant fatal collision, just like hitting the **Board** edge."

## Flagged ambiguities

- The class is called `Food`, but the variable instantiating it inside `Game` is named `fruit`. The term **Food** should be used consistently to avoid mixing "food" and "fruit" terminology.

## Code drift

Places where the code's name for a concept is not the canonical term. Reported, not renamed.

| Canonical term | Called in code | Location              | Note                                                          |
| -------------- | -------------- | --------------------- | ------------------------------------------------------------- |
| **Food**       | `fruit`        | `main.cpp:218`        | The variable inside `Game` is named `fruit` instead of `food` |
