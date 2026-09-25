// lab4/SMELLS.md — referenced in Part F of the report
// This is a local copy; the authoritative source is:
// https://it643.ankushchander.com/labs/lab4/SMELLS/

# Test Smells — short list (local reference)

| Smell | What it looks like |
|---|---|
| Assertion Roulette | Multiple `EXPECT_*` in one test with no message distinguishing which failed |
| Dead Test | Test is commented out or never runs |
| Eager Test | One test method tests many unrelated things |
| Irrelevant Information | Setup builds objects/state that the assertion never touches |
| Mystery Guest | Test reads from an external file or global without explanation |
| Test Without Assertion | Test exercises code but never asserts anything |
| Sensitive Equality | Uses exact string/float equality where slight changes break the test |
| Obsessive Constructor | Test spends more lines constructing than asserting |
