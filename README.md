# Chinese Checkers

A C++ implementation of Chinese Checkers with AI players. The board is a standard 6-pointed hex-star grid. Multiple AI strategies are available, and the game configuration (players, AI types, turn limit) is set interactively at startup.
Players are numbered 1–6; each player's goal is the opposite corner, making the opposing pairs: `1↔6`, `2↔5`, `3↔4`.

```
------------*------------
-----------*-*-----------
----------*-*-*----------
---------*-*-*-*---------
*-*-*-*-*-*-*-*-*-*-*-*-*
-*-*-*-*-*-*-*-*-*-*-*-*-
--*-*-*-*-*-*-*-*-*-*-*--
---*-*-*-*-*-*-*-*-*-*---
----*-*-*-*-*-*-*-*-*----
---*-*-*-*-*-*-*-*-*-*---
--*-*-*-*-*-*-*-*-*-*-*--
-*-*-*-*-*-*-*-*-*-*-*-*-
*-*-*-*-*-*-*-*-*-*-*-*-*
---------*-*-*-*---------
----------*-*-*----------
-----------*-*-----------
------------*------------
```

## Requirements

- macOS or Linux
- Clang or GCC with C++17 support (`clang++` / `g++`)
- `make` (optional, for the Makefile shortcut)

## Build

**Using make:**
```bash
make
```

**Manually:**
```bash
mkdir -p build
clang++ -std=c++17 -g -O0 -Wall -Wextra -Isrc -Isrc/player \
  $(find src -name '*.cpp') -o build/chinese_checkers
```

The binary is written to `build/chinese_checkers`.

## Run

```bash
./build/chinese_checkers
```

The program will prompt you to configure the game before it starts:

1. **Player IDs** — enter 2–6 space-separated IDs from the range `1`–`6`.  
   Opposing pairs share a start/goal corner: `1↔6`, `2↔5`, `3↔4`.  
   Common setups:
   | Players | IDs |
   |---------|-----|
   | 2-player | `1 4` |
   | 3-player | `1 3 5` |
   | 6-player | `1 2 3 4 5 6` |

2. **AI type per player**
   - `1` — GreedyAI: picks the locally best move each turn
   - `2` — MinimaxAI *(default)*: uses best-first minimax search with move ordering

3. **Max turns** — game ends when this limit is reached if no winner (default: `500`).

### Example session

```
=== Chinese Checkers Setup ===
Players are numbered 1-6 (opposing pairs: 1&6, 2&5, 3&4).
Common setups: 2 players ('1 4'), 3 players ('1 3 5'), 6 players ('1 2 3 4 5 6').

Enter player IDs (space-separated, e.g. '1 4'): 1 4

Select AI type for each player:
  [1] GreedyAI   [2] MinimaxAI

  Player 1 (default: 2 MinimaxAI): 1
  Player 4 (default: 2 MinimaxAI):

Max turns (default: 500): 200

Starting game with 2 players, max 200 turns.
```

## Project Structure

```
src/
  Main.cpp          # Entry point and game loop
  Game.cpp / .h     # Game orchestration, player management
  Board.cpp / .h    # Board state, move generation, win detection
  player/
    IPlayer.h       # Abstract player interface
    GreedyAI.cpp/h  # Greedy heuristic AI
    MinimaxAI.cpp/h # Minimax search AI
build/              # Compiled binary (git-ignored)
```
