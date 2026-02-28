#pragma once
#include "AIConfig.h"
#include "Board.h"
#include "IPlayer.h"
#include <list>
#include <map>
#include <vector>

namespace chinese_checkers {

/// Advanced AI using board-state evaluation with beam search.
///
/// Key improvements over MinimaxAI:
///   - Evaluates complete board positions (all 10 pieces) instead of
///     individual move deltas — captures straggler problems, deviation,
///     and target region progress holistically.
///   - Beam search prunes unpromising branches early, allowing deeper
///     effective search for comparable computation.
///   - O(1) move-ordering heuristic at inner nodes; full O(n) position
///     evaluation only at leaf nodes.
class BeamAI : public IPlayer {
public:
  BeamAI(int id, Board *board, const AIConfig &config = AIConfig::defaults())
      : id_(id), objectId_(7 - id), board_(board), config_(config) {}

  Move getNextMove() const override;

private:
  /// Evaluate the overall quality of a board position for this player.
  /// Considers total progress, straggler penalty, deviation, and regions.
  float evaluateBoard(const Board &board) const;

  /// Beam search: apply move, explore top-K children recursively,
  /// return the best leaf evaluation reachable from this move.
  float beamSearch(Board &board, const Move &move, int depth) const;

  /// Quick O(1) heuristic for move ordering (not used as final score).
  float quickMoveScore(const Move &move) const;

  /// Forward progress of a piece toward the target region (normalized).
  float getPieceProgress(const Position &pos) const;

  /// Lateral deviation of a piece from the main axis toward target.
  float getPieceDeviation(const Position &pos) const;

  /// Deduplicate moves: same from→to keeps the better variant.
  std::vector<Move> dedupMoves(const std::list<Move> &moves) const;

  int id_;
  int objectId_;
  Board *board_;
  AIConfig config_;
};

} // namespace chinese_checkers
