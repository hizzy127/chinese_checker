#pragma once
#include "AIConfig.h"
#include "Board.h"
#include "IPlayer.h"

namespace chinese_checkers {

class GreedyAI : public IPlayer {
public:
  GreedyAI(int id, Board *board, const AIConfig &config = AIConfig::defaults())
      : id_(id), objectId_(7 - id), board_(board), config_(config) {};
  Move getNextMove() const override;

private:
  int getMoveScore(const Move &move) const;
  int id_;
  int objectId_;
  Board *board_;
  AIConfig config_;
};

} // namespace chinese_checkers
