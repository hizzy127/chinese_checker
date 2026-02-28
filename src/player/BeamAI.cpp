#include "BeamAI.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <vector>

namespace chinese_checkers {

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------

Move BeamAI::getNextMove() const {
  auto startTime = std::chrono::high_resolution_clock::now();

  auto rawMoves = board_->getAllAvailableMovesForPlayer(id_);
  auto moves = dedupMoves(rawMoves);
  if (moves.empty()) {
    throw std::runtime_error("No available moves for player " +
                             std::to_string(id_));
  }

  // Pre-sort by quick heuristic so parallel results are deterministic
  std::sort(moves.begin(), moves.end(), [this](const Move &a, const Move &b) {
    return quickMoveScore(a) > quickMoveScore(b);
  });

  // Filter out moves that pull a piece out of the target region —
  // once a piece reaches the goal area it should stay there.
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  std::vector<Move> filtered;
  filtered.reserve(moves.size());
  for (const auto &m : moves) {
    GridRegion fromR = board_->getRegionOfCoordinate(m.from);
    GridRegion toR = board_->getRegionOfCoordinate(m.to);
    if (fromR == targetRegion && toR != targetRegion)
      continue; // never leave the target
    filtered.push_back(m);
  }
  if (!filtered.empty())
    moves = std::move(filtered);

  // Parallel evaluation — each top-level move gets its own board clone
  float bestScore = -std::numeric_limits<float>::max();
  std::vector<Move> bestMoves;

  std::vector<std::future<std::pair<Move, float>>> futures;
  for (const auto &move : moves) {
    futures.emplace_back(std::async(
        std::launch::async, [this, move]() -> std::pair<Move, float> {
          Board boardCopy = board_->clone();
          return {move, beamSearch(boardCopy, move, config_.searchDepth)};
        }));
  }

  for (auto &future : futures) {
    auto [move, score] = future.get();
    if (score > bestScore) {
      bestScore = score;
      bestMoves.clear();
      bestMoves.push_back(move);
    } else if (score == bestScore) {
      bestMoves.push_back(move);
    }
  }

  // Random tie-break among equally-scored best moves
  Move bestMove;
  if (bestMoves.size() == 1) {
    bestMove = bestMoves[0];
  } else {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, bestMoves.size() - 1);
    bestMove = bestMoves[dist(rng)];
  }

  auto endTime = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
  std::cout << "BeamAI Player " << id_ << " chooses move with score "
            << bestScore << ": (" << bestMove.from.x << ", " << bestMove.from.y
            << ") to (" << bestMove.to.x << ", " << bestMove.to.y
            << ") [Time: " << duration.count() << "ms]" << std::endl;
  return bestMove;
}

// ---------------------------------------------------------------------------
// Beam search
// ---------------------------------------------------------------------------

float BeamAI::beamSearch(Board &board, const Move &move, int depth) const {
  Move reverseMove = {move.to, move.from};
  board.applyMove(move);

  // Immediate win — reward winning sooner (higher depth = found earlier)
  if (board.checkWin(id_)) {
    board.applyMove(reverseMove);
    return config_.winScore + static_cast<float>(depth) * 10.0f;
  }

  // Leaf: return full board evaluation
  if (depth <= 0) {
    float score = evaluateBoard(board);
    board.applyMove(reverseMove);
    return score;
  }

  // Generate, dedup, and order candidate moves by quick heuristic
  auto rawMoves = board.getAllAvailableMovesForPlayer(id_);
  auto candidates = dedupMoves(rawMoves);

  if (candidates.empty()) {
    // No legal moves from this position (shouldn't normally happen)
    float score = evaluateBoard(board);
    board.applyMove(reverseMove);
    return score;
  }

  std::sort(candidates.begin(), candidates.end(),
            [this](const Move &a, const Move &b) {
              return quickMoveScore(a) > quickMoveScore(b);
            });

  // Beam: evaluate only top-K candidates
  int beamWidth =
      std::min(config_.maxBranch, static_cast<int>(candidates.size()));

  float bestScore = -std::numeric_limits<float>::max();
  for (int i = 0; i < beamWidth; ++i) {
    float score = beamSearch(board, candidates[i], depth - 1);
    bestScore = std::max(bestScore, score);
  }

  board.applyMove(reverseMove);
  return bestScore;
}

// ---------------------------------------------------------------------------
// Board-state evaluation  (the key improvement over MinimaxAI)
// ---------------------------------------------------------------------------

float BeamAI::evaluateBoard(const Board &board) const {
  auto pieces = board.getPiecesForPlayer(id_);
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  GridRegion homeRegion = static_cast<GridRegion>(id_);

  float totalProgress = 0.0f;
  float minProgress = std::numeric_limits<float>::max();
  float totalDeviation = 0.0f;
  int piecesInTarget = 0;
  int piecesInWrongRegion = 0;
  float settledBonus = 0.0f; // extra reward for pieces deep in the target

  for (const auto &pos : pieces) {
    float progress = getPieceProgress(pos);
    totalProgress += progress;
    minProgress = std::min(minProgress, progress);
    totalDeviation += std::abs(getPieceDeviation(pos));

    GridRegion region = board.getRegionOfCoordinate(pos);
    if (region == targetRegion) {
      piecesInTarget++;
      // Pieces deeper in the target get a larger bonus so they don't move out.
      // getPieceProgress already increases toward target, so use it directly.
      settledBonus += progress;
    } else if (region != homeRegion && region != GridRegion::Public) {
      piecesInWrongRegion++;
    }
  }

  float avgProgress = totalProgress / static_cast<float>(pieces.size());

  float score = 0.0f;
  score += config_.progressWeight * totalProgress;
  score -= config_.stragglerWeight * (avgProgress - minProgress);
  score -= config_.deviationPenalty * totalDeviation;
  score += config_.targetRegionBonus * static_cast<float>(piecesInTarget);
  score -= config_.wrongRegionPenalty * static_cast<float>(piecesInWrongRegion);
  // Strong bonus for pieces deep inside the target — discourages shuffling
  score += config_.targetRegionBonus * 0.5f * settledBonus;

  return score;
}

// ---------------------------------------------------------------------------
// Quick move-ordering heuristic  O(1) per move
// ---------------------------------------------------------------------------

float BeamAI::quickMoveScore(const Move &move) const {
  float progressDelta = getPieceProgress(move.to) - getPieceProgress(move.from);
  float deviationDelta = std::abs(getPieceDeviation(move.to)) -
                         std::abs(getPieceDeviation(move.from));

  float score = progressDelta - config_.deviationPenalty * deviationDelta;

  // Hops cover more ground — slight bonus for multi-hop chains
  if (move.isHop)
    score += 0.5f * static_cast<float>(move.numHops);

  // Region-based adjustments
  GridRegion toRegion = board_->getRegionOfCoordinate(move.to);
  GridRegion fromRegion = board_->getRegionOfCoordinate(move.from);
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  GridRegion homeRegion = static_cast<GridRegion>(id_);

  if (toRegion == targetRegion)
    score += config_.targetRegionBonus;
  if (toRegion != homeRegion && toRegion != targetRegion &&
      toRegion != GridRegion::Public)
    score -= config_.wrongRegionPenalty;

  // Heavily penalize moving a piece OUT of the target region
  if (fromRegion == targetRegion && toRegion != targetRegion)
    score -= config_.wrongRegionPenalty * 2.0f;

  return score;
}

// ---------------------------------------------------------------------------
// Piece metrics
// ---------------------------------------------------------------------------

float BeamAI::getPieceProgress(const Position &pos) const {
  // Forward progress toward the target region, normalized so all directions
  // advance by ~1 per single step.
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  switch (targetRegion) {
  case GridRegion::Region1:
    return static_cast<float>(pos.y); // Player 6 → up
  case GridRegion::Region6:
    return static_cast<float>(-pos.y); // Player 1 → down
  case GridRegion::Region2:
    return static_cast<float>(pos.x + pos.y) / 2.0f; // Player 5 → upper-right
  case GridRegion::Region5:
    return static_cast<float>(-(pos.x + pos.y)) / 2.0f; // Player 2 → lower-left
  case GridRegion::Region3:
    return static_cast<float>(pos.x - pos.y) / 2.0f; // Player 4 → lower-right
  case GridRegion::Region4:
    return static_cast<float>(pos.y - pos.x) / 2.0f; // Player 3 → upper-left
  default:
    return 0.0f;
  }
}

float BeamAI::getPieceDeviation(const Position &pos) const {
  // Lateral deviation from the main axis toward target (perpendicular
  // component).  Lower is better.
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  switch (targetRegion) {
  case GridRegion::Region1:
  case GridRegion::Region6:
    return static_cast<float>(std::abs(pos.x)); // deviation from x=0
  case GridRegion::Region2:
  case GridRegion::Region5:
    return static_cast<float>(std::abs(pos.x - pos.y)) /
           2.0f; // perp to x+y axis
  case GridRegion::Region3:
  case GridRegion::Region4:
    return static_cast<float>(std::abs(pos.x + pos.y)) /
           2.0f; // perp to x-y axis
  default:
    return 0.0f;
  }
}

// ---------------------------------------------------------------------------
// Move deduplication
// ---------------------------------------------------------------------------

std::vector<Move> BeamAI::dedupMoves(const std::list<Move> &moves) const {
  std::map<std::pair<Position, Position>, Move> uniqueMoves;

  for (const auto &move : moves) {
    auto key = std::make_pair(move.from, move.to);
    if (uniqueMoves.find(key) == uniqueMoves.end()) {
      uniqueMoves[key] = move;
    } else {
      const Move &existing = uniqueMoves[key];
      // Prefer hops and longer hop chains
      if (move.isHop && existing.isHop && move.numHops > existing.numHops) {
        uniqueMoves[key] = move;
      } else if (move.isHop && !existing.isHop) {
        uniqueMoves[key] = move;
      }
    }
  }

  std::vector<Move> result;
  result.reserve(uniqueMoves.size());
  for (auto &[key, move] : uniqueMoves) {
    result.push_back(std::move(move));
  }
  return result;
}

} // namespace chinese_checkers
