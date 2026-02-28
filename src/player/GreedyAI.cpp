#include "GreedyAI.h"
#include <random>

namespace chinese_checkers {

Move GreedyAI::getNextMove() const {
  auto moves = board_->getAllAvailableMovesForPlayer(id_);
  if (moves.empty()) {
    throw std::runtime_error("No available moves for player " +
                             std::to_string(id_));
  }
  // Find the highest score
  int bestScore = std::numeric_limits<int>::min();
  std::vector<Move> bestMoves;
  for (const auto &move : moves) {
    int score = getMoveScore(move);
    if (score > bestScore) {
      bestScore = score;
      bestMoves.clear();
      bestMoves.push_back(move);
    } else if (score == bestScore) {
      bestMoves.push_back(move);
    }
  }
  // Pick randomly among bestMoves
  if (bestMoves.size() == 1) {
    return bestMoves[0];
  } else {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, bestMoves.size() - 1);
    return bestMoves[dist(rng)];
  }
}

int GreedyAI::getMoveScore(const Move &move) const {
  int verticalMove = move.to.y - move.from.y;
  int upRightMove = (move.to.x + move.to.y - (move.from.x + move.from.y)) / 2;
  int upLeftMove = (move.to.y - move.to.x - (move.from.y - move.from.x)) / 2;
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  switch (targetRegion) {
  case GridRegion::Region1:
    return verticalMove;
  case GridRegion::Region2:
    return upRightMove;
  case GridRegion::Region3:
    return -upLeftMove;
  case GridRegion::Region4:
    return upLeftMove;
  case GridRegion::Region5:
    return -upRightMove;
  case GridRegion::Region6:
    return -verticalMove;
  default:
    return -1;
  }
}
} // namespace chinese_checkers
