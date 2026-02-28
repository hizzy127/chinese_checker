#include "Game.h"
#include "player/AIConfig.h"
#include "player/GreedyAI.h"
#include "player/MinimaxAI.h"
#include <iostream>
#include <memory>
#include <stdexcept>

namespace chinese_checkers {

Game::Game(const std::vector<int> &players)
    : board_(players), numPlayers_(static_cast<int>(players.size())),
      gameOver_(false), winner_(-1) {
  std::cout << "Board initialized with " << numPlayers_ << " players."
            << std::endl;
  for (int p : players) {
    std::cout << "Player " << p << " in game.\n";
    playerObjects_.emplace_back(std::make_unique<MinimaxAI>(p, &board_));
  }
}

Game::Game(const std::vector<std::pair<int, PlayerType>> &playerConfigs) {
  std::vector<int> playerIds;
  playerIds.reserve(playerConfigs.size());
  for (auto &cfg : playerConfigs)
    playerIds.push_back(cfg.first);

  // Re-initialise via placement: reconstruct members manually
  board_ = Board(playerIds);
  numPlayers_ = static_cast<int>(playerConfigs.size());
  gameOver_ = false;
  winner_ = -1;

  std::cout << "Board initialized with " << numPlayers_ << " players."
            << std::endl;
  for (auto &[id, type] : playerConfigs) {
    const char *typeName =
        (type == PlayerType::GreedyAI) ? "GreedyAI" : "MinimaxAI";
    std::cout << "Player " << id << " in game (" << typeName << ").\n";
    if (type == PlayerType::GreedyAI)
      playerObjects_.emplace_back(std::make_unique<GreedyAI>(id, &board_));
    else
      playerObjects_.emplace_back(std::make_unique<MinimaxAI>(id, &board_));
  }
  players_ = playerIds;
}

Game::Game(const std::vector<PlayerSetup> &playerSetups) {
  std::vector<int> playerIds;
  playerIds.reserve(playerSetups.size());
  for (auto &ps : playerSetups)
    playerIds.push_back(ps.id);

  board_ = Board(playerIds);
  numPlayers_ = static_cast<int>(playerSetups.size());
  gameOver_ = false;
  winner_ = -1;

  std::cout << "Board initialized with " << numPlayers_ << " players."
            << std::endl;
  for (auto &ps : playerSetups) {
    const char *typeName =
        (ps.type == PlayerType::GreedyAI) ? "GreedyAI" : "MinimaxAI";
    std::cout << "Player " << ps.id << " in game (" << typeName;
    if (ps.type == PlayerType::MinimaxAI) {
      std::cout << ", depth=" << ps.aiConfig.searchDepth;
    }
    std::cout << ").\n";
    if (ps.type == PlayerType::GreedyAI)
      playerObjects_.emplace_back(
          std::make_unique<GreedyAI>(ps.id, &board_, ps.aiConfig));
    else
      playerObjects_.emplace_back(
          std::make_unique<MinimaxAI>(ps.id, &board_, ps.aiConfig));
  }
  players_ = playerIds;
}

bool Game::makeMove(const Move &move) { return board_.applyMove(move); }
std::list<Move> Game::getValidMoves() const {
  return board_.getAllAvailableMovesForPlayer(board_.getCurrentPlayer());
}

Move Game::getNextMoveForCurrentPlayer() const {
  return playerObjects_[board_.getCurrentPlayerIndex()]->getNextMove();
}

int Game::getCurrentPlayer() const { return board_.getCurrentPlayer(); }

bool Game::isGameOver() const { return gameOver_; }

int Game::getWinner() const { return winner_; }

const Board &Game::getBoard() const { return board_; }

void Game::reset() {
  board_.reset(board_.getPlayers());
  gameOver_ = false;
  winner_ = -1;
}

void Game::printBoard() const { board_.printGrid(); }

void Game::endTurn() {
  int currentPlayer = board_.getCurrentPlayer();
  if (board_.checkWin(currentPlayer)) {
    gameOver_ = true;
    winner_ = currentPlayer;
    std::cout << "Player " << winner_ << " wins!\n";
    return;
  }
  board_.nextPlayer();
  std::cout << "Now it's Player " << board_.getCurrentPlayer() << "'s turn.\n";
}

} // namespace chinese_checkers