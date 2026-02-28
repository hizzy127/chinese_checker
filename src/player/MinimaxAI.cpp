#include "MinimaxAI.h"
#include <chrono>
#include <future>
#include <iostream>
#include <map>
#include <random>
#include <vector>

namespace chinese_checkers {

const int SEARCH_DEPTH = 3;
const float DEVIATION_PENALTY = 0.5f;

Move MinimaxAI::getNextMove() const {
  auto start_time = std::chrono::high_resolution_clock::now();

  auto rawMoves = board_->getAllAvailableMovesForPlayer(id_);
  auto moves = dedupMoves(rawMoves);
  if (moves.empty()) {
    throw std::runtime_error("No available moves for player " +
                             std::to_string(id_));
  }
  // Find the highest score using parallel evaluation
  float bestScore = std::numeric_limits<float>::min();
  std::vector<Move> bestMoves;

  // Launch async tasks for move evaluation
  std::vector<std::future<std::pair<Move, float>>> futures;
  for (const auto &move : moves) {
    futures.emplace_back(std::async(
        std::launch::async, [this, move]() -> std::pair<Move, float> {
          // Create a deep copy of the board for this thread
          Board boardCopy = board_->clone();
          return {move, evaluateMoveOnBoard(boardCopy, move, SEARCH_DEPTH)};
        }));
  }

  // Collect results from parallel tasks
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

  Move bestMove;
  // Pick randomly among bestMoves
  if (bestMoves.size() == 1) {
    bestMove = bestMoves[0];
  } else {
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, bestMoves.size() - 1);
    bestMove = bestMoves[dist(rng)];
  }
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  std::cout << "MinimaxAI Player " << id_ << " chooses move with score "
            << bestScore << ": (" << bestMove.from.x << ", " << bestMove.from.y
            << ") to (" << bestMove.to.x << ", " << bestMove.to.y
            << ") [Time: " << duration.count() << "ms]" << std::endl;
  return bestMove;
}

float MinimaxAI::evaluateMove(const Move &move, int depth) const {
  float baseScore = getMoveScore(move);
  Move reverseMove = {move.to, move.from};

  bool isMaximizing = board_->getCurrentPlayer() == id_;

  board_->applyMove(move);
  if (board_->checkWin(id_ - 1)) {
    // std::cout << "MinimaxAI Player " << id_ << " found winning move: ("
    //           << move.from.x << ", " << move.from.y << ") to (" << move.to.x
    //           << ", " << move.to.y << "). Current depth: " << depth
    //           << std::endl;
    board_->applyMove(reverseMove); // undo move
    return 1000.0f;                 // Winning move
  }

  if (depth <= 0 || baseScore < -2.0f) {
    board_->applyMove(reverseMove); // undo move
    return baseScore;
  }

  auto rawMoves = board_->getAllAvailableMovesForPlayer(id_);
  auto availableMoves = dedupMoves(rawMoves);
  float nextMoveScore = 0.0f;
  // Simulate each available move and evaluate the board state
  for (const auto &nextMove : availableMoves) {
    nextMoveScore = std::max(evaluateMove(nextMove, depth - 1), nextMoveScore);
  }
  board_->applyMove(reverseMove); // Undo move

  return isMaximizing ? baseScore + 0.9f * nextMoveScore
                      : baseScore - 0.3f * nextMoveScore;
}

float MinimaxAI::evaluateMoveOnBoard(Board &boardCopy, const Move &move,
                                     int depth) const {
  float baseScore = getMoveScore(move);
  Move reverseMove = {move.to, move.from};

  boardCopy.applyMove(move);
  if (boardCopy.checkWin(id_)) {
    boardCopy.applyMove(reverseMove); // undo move
    return 1000.0f;                   // Winning move
  }

  if (depth <= 0 || baseScore < -2.0f) {
    boardCopy.applyMove(reverseMove); // undo move
    return baseScore;
  }

  auto rawMoves = boardCopy.getAllAvailableMovesForPlayer(id_);
  auto availableMoves = dedupMoves(rawMoves);
  float nextMoveScore = 0.0f;
  // Simulate each available move and evaluate the board state
  for (const auto &nextMove : availableMoves) {
    nextMoveScore = std::max(
        evaluateMoveOnBoard(boardCopy, nextMove, depth - 1), nextMoveScore);
  }
  boardCopy.applyMove(reverseMove); // Undo move
  return baseScore + 0.9f * nextMoveScore;
}

float MinimaxAI::bestFirstSearch(Board &boardCopy, const Move &move,
                                 int depth) const {
  float baseScore = getMoveScore(move);
  Move reverseMove = {move.to, move.from};

  boardCopy.applyMove(move);

  // Check for winning condition
  if (boardCopy.checkWin(id_)) {
    boardCopy.applyMove(reverseMove);
    return 1000.0f + baseScore; // High bonus for winning
  }

  if (depth <= 0) {
    boardCopy.applyMove(reverseMove);
    return baseScore;
  }

  // Get, deduplicate, and order moves for better search efficiency
  auto availableMovesRaw = boardCopy.getAllAvailableMovesForPlayer(id_);
  auto dedupedMoves = dedupMoves(availableMovesRaw);
  auto availableMoves =
      orderMoves(std::list<Move>(dedupedMoves.begin(), dedupedMoves.end()));

  float bestFutureScore = 0.0f;
  int movesEvaluated = 0;
  const int maxMovesToEvaluate =
      std::min(20, (int)availableMoves.size()); // Limit branching

  for (const auto &nextMove : availableMoves) {
    if (movesEvaluated >= maxMovesToEvaluate)
      break;

    float futureScore = bestFirstSearch(boardCopy, nextMove, depth - 1);
    bestFutureScore = std::max(bestFutureScore, futureScore);
    movesEvaluated++;
  }

  boardCopy.applyMove(reverseMove);
  return baseScore + 0.8f * bestFutureScore; // Discount future moves
}

std::vector<Move> MinimaxAI::orderMoves(const std::list<Move> &moves) const {
  std::vector<Move> orderedMoves(moves.begin(), moves.end());

  // Sort moves by priority: hops first, then by score
  std::sort(orderedMoves.begin(), orderedMoves.end(),
            [this](const Move &a, const Move &b) {
              // Prioritize hops (they're often better)
              if (a.isHop && !b.isHop)
                return true;
              if (!a.isHop && b.isHop)
                return false;

              // For hops, prefer longer jumps
              if (a.isHop && b.isHop) {
                if (a.numHops != b.numHops)
                  return a.numHops > b.numHops;
              }

              // Otherwise, sort by move score
              return getMoveScore(a) > getMoveScore(b);
            });

  return orderedMoves;
}

std::vector<Move> MinimaxAI::dedupMoves(const std::list<Move> &moves) const {
  std::map<std::pair<Position, Position>, Move> uniqueMoves;

  for (const auto &move : moves) {
    auto key = std::make_pair(move.from, move.to);

    // If this from->to combination doesn't exist yet, add it
    if (uniqueMoves.find(key) == uniqueMoves.end()) {
      uniqueMoves[key] = move;
    } else {
      // If it exists, keep the one with higher score or more hops
      const Move &existing = uniqueMoves[key];

      // Prefer moves with more hops (longer jumps are often better)
      if (move.isHop && existing.isHop && move.numHops > existing.numHops) {
        uniqueMoves[key] = move;
      } else if (move.isHop && !existing.isHop) {
        // Prefer hops over regular moves
        uniqueMoves[key] = move;
      } else if (!move.isHop && !existing.isHop) {
        // For regular moves, prefer the one with higher score
        if (getMoveScore(move) > getMoveScore(existing)) {
          uniqueMoves[key] = move;
        }
      }
      // If existing is hop and new is not, keep existing
    }
  }

  // Convert map back to vector
  std::vector<Move> dedupedMoves;
  for (const auto &pair : uniqueMoves) {
    dedupedMoves.push_back(pair.second);
  }

  return dedupedMoves;
}

int MinimaxAI::getMoveScore(const Move &move) const {
  int verticalMove = move.to.y - move.from.y;
  int upRightMove = (move.to.x + move.to.y - (move.from.x + move.from.y)) / 2;
  int upLeftMove = (move.to.y - move.to.x - (move.from.y - move.from.x)) / 2;
  GridRegion targetRegion = static_cast<GridRegion>(objectId_);
  GridRegion homeRegion = static_cast<GridRegion>(id_);
  GridRegion toRegion = board_->getRegionOfCoordinate(move.to);
  GridRegion fromRegion = board_->getRegionOfCoordinate(move.from);

  float score = 0.0f;

  if (toRegion != homeRegion && toRegion != GridRegion::Public &&
      toRegion != targetRegion) {
    // Penalize moving out of home region unless it's to the public region
    score -= 10.0f;
  }

  float toDeviation = 0.0f;
  float fromDeviation = 0.0f;
  switch (targetRegion) {
  case GridRegion::Region1:
  case GridRegion::Region6:
    toDeviation = std::abs(move.to.x);
    fromDeviation = std::abs(move.from.x);
    break;
  case GridRegion::Region2:
  case GridRegion::Region5:
    toDeviation = std::abs(move.to.x + move.to.y);
    fromDeviation = std::abs(move.from.x + move.from.y);
    break;
  case GridRegion::Region3:
  case GridRegion::Region4:
    toDeviation = std::abs(move.to.y - move.to.x);
    fromDeviation = std::abs(move.from.y - move.from.x);
    break;
  default:
    break;
  }

  if (toDeviation > fromDeviation) {
    score -= DEVIATION_PENALTY * (toDeviation - fromDeviation);
  }

  float moveTowardTarget = 0.0f;
  switch (targetRegion) {
  case GridRegion::Region1:
    moveTowardTarget += verticalMove;
    break;
  case GridRegion::Region2:
    moveTowardTarget += upRightMove;
    break;
  case GridRegion::Region3:
    moveTowardTarget -= upLeftMove;
    break;
  case GridRegion::Region4:
    moveTowardTarget += upLeftMove;
    break;
  case GridRegion::Region5:
    moveTowardTarget -= upRightMove;
    break;
  case GridRegion::Region6:
    moveTowardTarget -= verticalMove;
    break;
  default:
    moveTowardTarget -= 1;
  }
  score += moveTowardTarget;

  if (fromRegion == homeRegion && moveTowardTarget > 0) {
    score += 0.5f * moveTowardTarget;
    // score += 1.5f;
  }
  return score;
}
} // namespace chinese_checkers
