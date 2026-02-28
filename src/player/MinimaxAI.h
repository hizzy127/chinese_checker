#pragma once
#include "AIConfig.h"
#include "Board.h"
#include "IPlayer.h"

namespace chinese_checkers {

class MinimaxAI : public IPlayer {
public:
  MinimaxAI(int id, Board *board, const AIConfig &config = AIConfig::defaults())
      : id_(id), objectId_(7 - id), board_(board), config_(config) {};
  Move getNextMove() const override;

private:
  int getMoveScore(const Move &move) const;
  float evaluateMove(const Move &move, int depth) const;
  float evaluateMoveOnBoard(Board &boardCopy, const Move &move,
                            int depth) const;
  float bestFirstSearch(Board &boardCopy, const Move &move, int depth) const;
  std::vector<Move> orderMoves(const std::list<Move> &moves) const;
  std::vector<Move> dedupMoves(const std::list<Move> &moves) const;
  int id_;
  int objectId_;
  Board *board_;
  AIConfig config_;
};

} // namespace chinese_checkers
