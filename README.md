# THE LAST TRAIN — CS-1002 Programming Fundamentals Project

**Student Roll Numbers:** `260106` & `266519`  
**Repository:** `PF-Project-TheLastTrain-260106-266519`  
**Constraint:** 100% Procedural C++17 — **No Classes and No Structs** (parallel arrays & primitive references).

## Build & Run (From Repository Root)
1. Compile: `g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train`
2. Interactive Run: `./last_train`
3. Victory Verification: `./last_train < data/test_win.txt && cat out/game_summary.txt`
4. Determinism Check: `./last_train < data/test_win.txt > /dev/null && cp out/game_summary.txt /tmp/r1.txt && ./last_train < data/test_win.txt > /dev/null && diff -u /tmp/r1.txt out/game_summary.txt`
