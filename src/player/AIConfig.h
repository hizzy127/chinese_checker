#pragma once

#include <string>

namespace chinese_checkers {

/// Configurable parameters for AI players.
/// Each player instance can have its own config.
struct AIConfig {
  // --- MinimaxAI parameters ---
  int searchDepth = 3;
  float deviationPenalty = 0.5f;
  float futureDiscount =
      0.9f;                 // discount for future moves in evaluateMoveOnBoard
  float bfsDiscount = 0.8f; // discount for future moves in bestFirstSearch
  float opponentDiscount = 0.3f; // discount for opponent moves in evaluateMove
  int maxBranch = 20; // max moves to evaluate per level in bestFirstSearch
  float winScore = 1000.0f;      // score assigned to a winning move
  float baseScoreCutoff = -2.0f; // prune moves with base score below this
  float homeRegionBonus = 0.5f;  // bonus multiplier for moves from home region
  float wrongRegionPenalty = 10.0f; // penalty for moving into wrong region

  /// Returns a config with all default values.
  static AIConfig defaults() { return AIConfig{}; }
};

} // namespace chinese_checkers
