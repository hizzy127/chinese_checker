#pragma once
#include "Board.h"
#include "IPlayer.h"

namespace chinese_checkers {

class GreedyAI : public IPlayer {
public:
  GreedyAI(int id, Board *board) : id_(id), objectId_(7 - id), board_(board) {};
  Move getNextMove() const override;

private:
  int getMoveScore(const Move &move) const;
  int id_;
  int objectId_;
  Board *board_;
};

} // namespace chinese_checkers
