#pragma once

#include "player/AIConfig.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace chinese_checkers {

enum class PlayerType; // forward-declared; defined in Game.h

/// Holds the full game configuration loaded from a config file.
struct GameFileConfig {
  struct PlayerConfig {
    int id = 0;
    std::string type = "MinimaxAI"; // "GreedyAI" or "MinimaxAI"
    AIConfig aiConfig;
  };

  std::vector<PlayerConfig> players;
  int maxTurns = 500;
};

/// Simple INI-style config file parser.
///
/// File format example:
/// ```
/// [game]
/// max_turns = 500
///
/// [player.1]
/// type = MinimaxAI
/// search_depth = 4
/// deviation_penalty = 0.3
///
/// [player.6]
/// type = GreedyAI
/// ```
class ConfigParser {
public:
  /// Parse a config file and return the configuration.
  /// Returns true on success, false on failure.
  static bool parse(const std::string &filePath, GameFileConfig &out) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
      std::cerr << "Could not open config file: " << filePath << "\n";
      return false;
    }

    std::string currentSection;
    std::map<std::string, std::map<std::string, std::string>> sections;

    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
      lineNum++;
      // Trim whitespace
      line = trim(line);

      // Skip empty lines and comments
      if (line.empty() || line[0] == '#' || line[0] == ';')
        continue;

      // Section header
      if (line.front() == '[' && line.back() == ']') {
        currentSection = line.substr(1, line.size() - 2);
        currentSection = trim(currentSection);
        continue;
      }

      // Key = Value
      auto eqPos = line.find('=');
      if (eqPos == std::string::npos) {
        std::cerr << "Config parse error at line " << lineNum
                  << ": missing '='\n";
        continue;
      }

      std::string key = trim(line.substr(0, eqPos));
      std::string value = trim(line.substr(eqPos + 1));
      sections[currentSection][key] = value;
    }

    // Process [game] section
    if (sections.count("game")) {
      auto &gameSection = sections["game"];
      if (gameSection.count("max_turns"))
        out.maxTurns = std::stoi(gameSection["max_turns"]);
    }

    // Process [player.N] sections
    for (auto &[sectionName, kvs] : sections) {
      if (sectionName.rfind("player.", 0) != 0)
        continue; // not a player section

      std::string idStr = sectionName.substr(7); // after "player."
      int playerId = 0;
      try {
        playerId = std::stoi(idStr);
      } catch (...) {
        std::cerr << "Config: invalid player section [" << sectionName << "]\n";
        continue;
      }

      if (playerId < 1 || playerId > 6) {
        std::cerr << "Config: player ID must be 1-6, got " << playerId << "\n";
        continue;
      }

      GameFileConfig::PlayerConfig pc;
      pc.id = playerId;

      if (kvs.count("type"))
        pc.type = kvs["type"];

      // Parse AIConfig fields
      if (kvs.count("search_depth"))
        pc.aiConfig.searchDepth = std::stoi(kvs["search_depth"]);
      if (kvs.count("deviation_penalty"))
        pc.aiConfig.deviationPenalty = std::stof(kvs["deviation_penalty"]);
      if (kvs.count("future_discount"))
        pc.aiConfig.futureDiscount = std::stof(kvs["future_discount"]);
      if (kvs.count("bfs_discount"))
        pc.aiConfig.bfsDiscount = std::stof(kvs["bfs_discount"]);
      if (kvs.count("opponent_discount"))
        pc.aiConfig.opponentDiscount = std::stof(kvs["opponent_discount"]);
      if (kvs.count("max_branch"))
        pc.aiConfig.maxBranch = std::stoi(kvs["max_branch"]);
      if (kvs.count("win_score"))
        pc.aiConfig.winScore = std::stof(kvs["win_score"]);
      if (kvs.count("base_score_cutoff"))
        pc.aiConfig.baseScoreCutoff = std::stof(kvs["base_score_cutoff"]);
      if (kvs.count("home_region_bonus"))
        pc.aiConfig.homeRegionBonus = std::stof(kvs["home_region_bonus"]);
      if (kvs.count("wrong_region_penalty"))
        pc.aiConfig.wrongRegionPenalty = std::stof(kvs["wrong_region_penalty"]);

      out.players.push_back(pc);
    }

    // Sort players by ID for consistent ordering
    std::sort(
        out.players.begin(), out.players.end(),
        [](const GameFileConfig::PlayerConfig &a,
           const GameFileConfig::PlayerConfig &b) { return a.id < b.id; });

    return true;
  }

private:
  static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
      return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
  }
};

} // namespace chinese_checkers
