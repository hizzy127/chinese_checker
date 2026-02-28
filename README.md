# Chinese Checkers

A C++ implementation of Chinese Checkers with AI players. The board is a standard 6-pointed hex-star grid. Multiple AI strategies are available, and the game configuration (players, AI types, AI parameters, turn limit) can be set either via a config file or interactively at startup.
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

### Interactive mode

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

### Config file mode

*The `BeamAI` (formerly AdvancedAI) uses beam search — see docs below.*

```bash
./build/chinese_checkers config.ini
```

Pass an INI config file as the first argument to skip the interactive prompt and configure every player's AI parameters individually. If the file cannot be loaded the program falls back to the interactive prompt.

## Config file format

The config file uses a simple INI format with a `[game]` section and one `[player.N]` section per player.

```ini
[game]
max_turns = 500

[player.1]
type = MinimaxAI
search_depth = 4
deviation_penalty = 0.3
future_discount = 0.9

[player.6]
type = GreedyAI
```

- Lines beginning with `#` or `;` are comments.
- Any parameter omitted from a section keeps its default value.
- Player IDs in the file determine which players are in the game (no interactive prompt is shown).

### Available parameters

| Parameter | Default | Applies to | Description |
|-----------|---------|------------|-------------|
| `type` | `MinimaxAI` | all | AI type: `MinimaxAI` or `GreedyAI` |
| `search_depth` | `3` | MinimaxAI | Minimax search depth |
| `deviation_penalty` | `0.5` | MinimaxAI | Penalty per unit of lateral deviation from the target axis |
| `future_discount` | `0.9` | MinimaxAI | Discount applied to future move scores |
| `bfs_discount` | `0.8` | MinimaxAI | Discount in best-first search |
| `opponent_discount` | `0.3` | MinimaxAI | Discount applied when computing opponent responses |
| `max_branch` | `20` | MinimaxAI | Max moves evaluated per level in best-first search |
| `win_score` | `1000.0` | MinimaxAI | Score assigned to a winning move |
| `base_score_cutoff` | `-2.0` | MinimaxAI | Prune branches whose base score is below this threshold |
| `home_region_bonus` | `0.5` | MinimaxAI | Bonus multiplier for moves that advance from the home region |
| `wrong_region_penalty` | `10.0` | MinimaxAI | Penalty for moving into an unrelated region |

A sample config is provided in [`config.ini`](config.ini).

## Project Structure

```
src/
  Main.cpp          # Entry point, game loop, config file loading
  Game.cpp / .h     # Game orchestration, player management
  Board.cpp / .h    # Board state, move generation, win detection
  ConfigParser.h    # INI config file parser and GameFileConfig types
  player/
    IPlayer.h       # Abstract player interface
    AIConfig.h      # AIConfig struct with all tunable AI parameters
    GreedyAI.cpp/h  # Greedy heuristic AI
    MinimaxAI.cpp/h # Minimax search AI
build/              # Compiled binary (git-ignored)
config.ini          # Sample config file
```
