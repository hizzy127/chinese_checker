#include "Game.h"
#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace chinese_checkers;

struct GameConfig {
  std::vector<std::pair<int, PlayerType>> playerConfigs;
  int maxTurns = 500;
};

static GameConfig promptConfig() {
  GameConfig cfg;

  std::cout << "\n=== Chinese Checkers Setup ===\n";
  std::cout << "Players are numbered 1-6 (opposing pairs: 1&6, 2&5, 3&4).\n";
  std::cout << "Common setups: 2 players ('1 6'), 3 players ('1 3 5'), "
               "6 players ('1 2 3 4 5 6').\n\n";

  // --- Select player IDs ---
  std::vector<int> playerIds;
  while (playerIds.empty()) {
    std::cout << "Enter player IDs (space-separated, e.g. '1 6'): ";
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    int id;
    std::set<int> seen;
    bool valid = true;
    while (iss >> id) {
      if (id < 1 || id > 6) {
        std::cout << "  Error: player IDs must be between 1 and 6.\n";
        valid = false;
        break;
      }
      if (!seen.insert(id).second) {
        std::cout << "  Error: duplicate player ID " << id << ".\n";
        valid = false;
        break;
      }
      playerIds.push_back(id);
    }
    if (!valid || playerIds.size() < 2) {
      if (valid)
        std::cout << "  Error: at least 2 players are required.\n";
      playerIds.clear();
    }
  }

  // --- Select AI type per player ---
  std::cout << "\nSelect AI type for each player:\n"
            << "  [1] GreedyAI   [2] MinimaxAI\n\n";
  for (int id : playerIds) {
    PlayerType type = PlayerType::MinimaxAI;
    while (true) {
      std::cout << "  Player " << id << " (default: 2 MinimaxAI): ";
      std::string line;
      std::getline(std::cin, line);
      if (line.empty() || line == "2") {
        type = PlayerType::MinimaxAI;
        break;
      }
      if (line == "1") {
        type = PlayerType::GreedyAI;
        break;
      }
      std::cout << "    Please enter 1 or 2.\n";
    }
    cfg.playerConfigs.emplace_back(id, type);
  }

  // --- Max turns ---
  std::cout << "\nMax turns (default: 500): ";
  std::string line;
  std::getline(std::cin, line);
  if (!line.empty()) {
    try {
      int val = std::stoi(line);
      if (val > 0)
        cfg.maxTurns = val;
    } catch (...) {
      std::cout << "  Invalid input, using default of 500.\n";
    }
  }

  return cfg;
}

int main() {
  GameConfig cfg = promptConfig();

  std::cout << "\nStarting game with " << cfg.playerConfigs.size()
            << " players, max " << cfg.maxTurns << " turns.\n\n";

  Game game(cfg.playerConfigs);

  int numTurns = 1;
  while (!game.isGameOver() && numTurns <= cfg.maxTurns) {
    auto moves = game.getValidMoves();
    if (moves.empty()) {
      std::cout << "No valid moves for Player " << game.getCurrentPlayer()
                << ". Skipping turn.\n";
      game.endTurn();
      numTurns++;
      continue;
    }
    auto move = game.getNextMoveForCurrentPlayer();
    game.makeMove(move);
    game.printBoard();
    game.endTurn();
    numTurns++;
  }

  if (game.isGameOver())
    std::cout << "\nGame over! Winner: Player " << game.getWinner()
              << ". Turns played: " << numTurns - 1 << "\n";
  else
    std::cout << "\nGame over! Reached the " << cfg.maxTurns
              << "-turn limit with no winner.\n";

  return 0;
}
