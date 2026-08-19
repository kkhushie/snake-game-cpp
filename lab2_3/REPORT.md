# Lab 2_3 — Group A__

---

## 1. Tool and install route — [3]

| | |
|---|---|
| Agent used for run 2 | Antigravity (Gemini CLI) |
| `ubiquitous-language` install route | project `skills/ubiquitous-language/SKILL.md` |
| `refactoring/` pack install route | project `skills/refactoring/` pack |

If anything would not install, say what failed here. The paste fallback is a documented
route and costs no marks.

---

## 2. What I changed in the glossary — [4]

The generated file is at `lab2_3/UBIQUITOUS_LANGUAGE.md`. Three or four lines here on what
you corrected and why: terms it invented, definitions it got wrong, ambiguities it missed.

I corrected the ambiguity between "Food" and "Fruit" (capitalizing them and making Fruit a specific type of Food). I also fixed the definition of "Game" to explicitly mention it holds exactly one Snake, and clarified that "Bomb" instantly terminates the game instead of just deducting score.

---

## 3. Smell delta — [6]

Reports: `lab2_3/audits/main.md` (the code as you received it) and `lab2_3/audits/lab1-head.md`
(after your Lab-1 PR).

| | count | representative site (`file:line`) |
|---|---|---|
| Smells my Lab-1 PR **introduced** | 2 | main.cpp:302 |
| Smells my Lab-1 PR **left untouched** | 2 | main.cpp:90 |
| Smells my Lab-1 PR **removed** | 0 | N/A |

The third row will probably be zero. Leave it in.

---

## 4. Rejected candidates — [6]

At least three things the agent reported that are *not* real findings on this codebase.

| smell reported | `file:line` | why it does not hold |
|---|---|---|
| Primitive Obsession | main.cpp:21 | Using raw `int x, y` instead of a `Point` struct is acceptable here to avoid overhead in a single file terminal game. |
| Switch Statements | main.cpp:295 | A switch to map user input to directions is standard, not an OO Abuser needing Command Pattern. |
| Magic Numbers | main.cpp:213 | Board dimensions and delay times are configuration constants specific to the terminal, not an architectural issue. |

---

## 5. Commit map — [7]

Run `lab2_3/check-lab2_3.sh` and paste the table it prints.

| # | sha | subject | what it is |
|---|---|---|---|
| 1 | 78bb453 | docs: Add Ubiquitous Language glossary | glossary |
| 2 | 4bafbd8 | docs: Add code smell report for main branch | smell report |
| 3 | f14bf00 | refactor: Consolidate snake tracking into a collection | **the refactor, alone** |
| 4 | e01e904 | feat: Add second player | **the feature, alone** |

---

## 6. Two-run measurement — [4]

Run 1 is your Lab-1 branch — the numbers you already reported. Run 2 is commit 4 alone.

| | Run 1 (Lab 1) | Run 2 (commit 4) |
|---|---|---|
| Smells introduced | 2 | 0 |
| Lines changed, `git diff --shortstat -w` | 134 | 107 |
| Lines changed, **raw** (no `-w`) | 148 | 107 |
| Functions reached | 6 | 4 |
| Prompts to working code | 4 | 2 |
| Wall-clock time | 40m | 10m |

Commit 3 (the refactor) on its own: 44 lines `-w`, 74 raw.

`check-lab2_3.sh` prints the four line-count numbers for run 2. Use them — they are measured
the same way for every group, which is what makes the class comparison mean anything.

---

## 7. Analysis Q1–Q2 — [5]

**Q1. Which smell did commit 3 actually fix?** Name it from your section 3 report. What was
expensive before, what does it cost now.

Commit 3 fixed the "Duplicated Variables / Data Clumps" smell (Parallel Logic). Before, adding a player cost duplicating every logic block and variable (`snake1`, `snake2`, `score1`, `score2`). Now, adding a player only costs changing `numSnakes` and tweaking the controls, as the data and logic are vectorized.

**Q2. Compare commit 4 to your Lab-1 diff.** Same feature, same codebase. What changed in
the cost and what did not? If it got worse, say so and explain — that marks the same.

In Commit 4, the cost of adding a second player drastically decreased. We didn't have to duplicate the collision logic or rendering loops, resulting in fewer lines changed and no new code smells introduced. The cost of manual input mapping (`W/A/S/D` vs `I/J/K/L`) remained the same, as the switch statement handles explicit keybindings for each player.

---

## 8. Analysis Q3–Q4 — [5]

**Q3. Go back through your Lab-1 `LLM-LOG.md`. Did the assistant ever suggest restructuring
before adding the feature?** Quote it if it did. If it did not, what would have had to be
different in your prompt?

The assistant did not suggest restructuring first. It immediately implemented the feature by duplicating variables to meet the prompt's requirements as fast as possible. To get restructuring first, the prompt would need an explicit design-first instruction like "Refactor the Game class to support arbitrary snakes via a collection before adding the second player's controls."

**Q4. How do you know commit 3 did not change behaviour?** Answer honestly. Most of you will
find that you do not know. Say that plainly if it is true, and describe what you would have
needed in order to actually know.

I honestly don't know for sure because this project lacks automated tests. We relied on manual compilation and visual inspection of the code delta. To actually know the behavior didn't change, we would need unit tests covering the `Game::logic()` loops and input handling, or integration tests verifying terminal state matches before and after the refactor.

---

Sections 7 and 8 together: **500 words maximum.**

## If you did not finish

Say so here and lose no marks. What you got to, where it broke, what you tried.

