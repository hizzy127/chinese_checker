#ifndef CHINESE_CHECKER_GAME_H
#define CHINESE_CHECKER_GAME_H

#include "Board.h"
#include "player/IPlayer.h"
#include <list>
#include <memory>
#include <utility>
#include <vector>

namespace chinese_checkers {

enum class PlayerType { GreedyAI, MinimaxAI };

class Game {
public:
  // Construct with player IDs only (all default to MinimaxAI)
  Game(const std::vector<int> &players);
  // Construct with per-player type configuration
  Game(const std::vector<std::pair<int, PlayerType>> &playerConfigs);
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
