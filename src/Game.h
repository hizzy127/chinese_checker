#ifndef CHINESE_CHECKER_GAME_H
#define CHINESE_CHECKER_GAME_H

#include "Board.h"
#include "player/AIConfig.h"
#include "player/IPlayer.h"
#include <list>
#include <memory>
#include <utility>
#include <vector>

namespace chinese_checkers {

enum class PlayerType { GreedyAI, MinimaxAI };

/// Per-player configuration: ID, AI type, and AI parameters.
struct PlayerSetup {
  int id;
  PlayerType type;
  AIConfig aiConfig;

  PlayerSetup(int id, PlayerType type,
              const AIConfig &cfg = AIConfig::defaults())
      : id(id), type(type), aiConfig(cfg) {}
};

class Game {
public:
  // Construct with player IDs only (all default to MinimaxAI with default
  // config)
  Game(const std::vector<int> &players);
  // Construct with per-player type configuration (backward compatible)
  Game(const std::vector<std::pair<int, PlayerType>> &playerConfigs);
  // Construct with full per-player setup (type + AI config)
  Game(const std::vector<PlayerSetup> &playerSetups);
  bool makeMove(const Move &move);
  std::list<Move> getValidMoves() const;
  Move getNextMoveForCurrentPlayer() const;
  int getCurrentPlayer() const;
  bool isGameOver() const;
  int getWinner() const;
  const Board &getBoard() const;
  void reset();
  void printBoard() const;
  void endTurn();

private:
  std::vector<int> players_;
  Board board_;
  int numPlayers_;
  std::vector<std::unique_ptr<IPlayer>> playerObjects_;
  bool gameOver_;
  int winner_;
};

} // namespace chinese_checkers

#endif // CHINESE_CHECKER_GAME_H
