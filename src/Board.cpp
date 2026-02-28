#include "Board.h"
#include <algorithm>
#include <iostream>
#include <list>

namespace chinese_checkers {

Board::Board() : currentPlayerIdx_(0) {
  players_ = {1, 2, 3, 4, 5, 6};
  initializeBoard();
}

Board::Board(std::vector<int> players)
    : players_(players), currentPlayerIdx_(0) {
  initializeBoard();
}

// Copy constructor
Board::Board(const Board &other)
    : grid_(other.grid_), players_(other.players_),
      currentPlayerIdx_(other.currentPlayerIdx_) {
  // Grid is copied by member initializer list
  // No additional initialization needed since grid_ contains all the state
}

// Copy assignment operator
Board &Board::operator=(const Board &other) {
  if (this != &other) {
    grid_ = other.grid_;
    players_ = other.players_;
    currentPlayerIdx_ = other.currentPlayerIdx_;
  }
  return *this;
}

// Deep copy method
Board Board::clone() const { return Board(*this); }

CellState getPlayer(int i) {
  switch (i) {
  case 1:
    return CellState::Player1;
  case 2:
    return CellState::Player2;
  case 3:
    return CellState::Player3;
  case 4:
    return CellState::Player4;
  case 5:
    return CellState::Player5;
  case 6:
    return CellState::Player6;
  default:
    return CellState::Invalid;
  }
}

void Board::initializeBoard() {
  currentPlayerIdx_ = 0;
  for (int x = 0; x <= BOARD_SIZE; ++x) {
    for (int y = 0; y <= BOARD_SIZE; ++y) {
      if (isValidPosition(getCoordinates({x, y}))) {
        grid_[x][y] = CellState::Empty;
      } else {
        grid_[x][y] = CellState::Invalid;
      }
    }
  }
  for (int i : players_) {
    initializeBoardForPlayer(i);
  }
  printGrid();
  std::cout << "Board initialized." << std::endl;
}

void Board::initializeBoardForPlayer(int player_idx) {
  for (int y = 0; y <= BOARD_SIZE; ++y) {
    for (int x = 0; x <= BOARD_SIZE; ++x) {
      if (isInStartRegionForPlayer(player_idx, getCoordinates({x, y})) &&
          grid_[x][y] != CellState::Invalid) {
        grid_[x][y] = getPlayer(player_idx);
      }
    }
  }
}

bool Board::isInStartRegionForPlayer(int playerIdx, Position coordinate) {
  return static_cast<int>(getRegionOfCoordinate(coordinate)) == playerIdx;
}

GridRegion Board::getRegionOfCoordinate(const Position &coordinate) const {
  if (!isValidPosition(coordinate)) {
    return GridRegion::Invalid;
  }
  if (coordinate.y > 4) {
    return GridRegion::Region1;
  }
  if (coordinate.y < -4) {
    return GridRegion::Region6;
  }
  if (coordinate.x + coordinate.y < -8) {
    return GridRegion::Region5;
  }
  if (coordinate.x + coordinate.y > 8) {
    return GridRegion::Region2;
  }
  if (coordinate.x - coordinate.y < -8) {
    return GridRegion::Region4;
  }
  if (coordinate.x - coordinate.y > 8) {
    return GridRegion::Region3;
  }
  return GridRegion::Public;
}

// Input should be a node in Public region
int Board::getDistanceToRegion(const Position &coordinate,
                               GridRegion region) const {
  if (getRegionOfCoordinate(coordinate) != GridRegion::Public) {
    std::cerr << "Input coordinate " << coordinate.x << ", " << coordinate.y
              << " not in Public region\n";
    return -1; // Invalid input
  }
  switch (region) {
  case GridRegion::Region1:
    return abs(4 - coordinate.y) + 1;
  case GridRegion::Region2:
    return abs(8 - coordinate.x - coordinate.y) / 2 + 1;
  case GridRegion::Region3:
    return abs(8 - coordinate.x + coordinate.y) / 2 + 1;
  case GridRegion::Region4:
    return abs(-8 - coordinate.x + coordinate.y) / 2 + 1;
  case GridRegion::Region5:
    return abs(-8 - coordinate.x - coordinate.y) / 2 + 1;
  case GridRegion::Region6:
    return abs(-4 - coordinate.y) + 1;
  default:
    return -1;
  }
}

void Board::printGrid() const {
  for (int y = BOARD_SIZE; y >= 0; --y) {
    for (int x = 0; x <= BOARD_SIZE; ++x) {
      if (grid_[x][y] == CellState::Invalid) {
        std::cout << "  ";
      } else if (grid_[x][y] == CellState::Empty) {
        std::cout << "* ";
        // auto coordinate = getCoordinates({x, y});
        // std::cout << getDistanceToRegion(coordinate, GridRegion::Region4)
        //           << " ";
      } else {
        std::cout << static_cast<int>(grid_[x][y]) << ' ';
      }
    }
    std::cout << y - BOARD_SIZE / 2 << '\n';
  }
}

CellState Board::getCell(const Position &pos) const {
  auto gridPos = getGridPosition(pos);
  return grid_[gridPos.x][gridPos.y];
}

void Board::setCell(const Position &pos, CellState state) {
  auto gridPos = getGridPosition(pos);
  grid_[gridPos.x][gridPos.y] = state;
}

bool Board::isValidPosition(const Position &pos) const {
  if ((abs(pos.x) + abs(pos.y)) % 2 == 1) {
    return false;
  }
  if ((abs(pos.x) + abs(pos.y)) > 8 && abs(pos.y) > 4) {
    return false;
  }
  if ((abs(pos.x) + pos.y) > 8 && (abs(pos.x) - pos.y) > 8) {
    return false;
  }
  return true;
}

std::vector<Position> Board::getPlayerPositions(int player) const {
  std::vector<Position> positions;
  CellState state = static_cast<CellState>(player);
  for (int x = 0; x <= BOARD_SIZE; ++x) {
    for (int y = 0; y <= BOARD_SIZE; ++y) {
      if (grid_[x][y] == state) {
        positions.push_back(getCoordinates({x, y}));
      }
    }
  }
  return positions;
}

bool Board::applyMove(const Move &move) {
  // Apply the move to the board
  const auto fromState = getCell(move.from);
  const auto toState = getCell(move.to);
  if (fromState == CellState::Invalid || fromState == CellState::Empty ||
      toState == CellState::Invalid || toState != CellState::Empty) {
    std::cerr << "Attempting to apply move with invalid positions. move: ("
              << move.from.x << ", " << move.from.y << ") to (" << move.to.x
              << ", " << move.to.y << ")\n";
    return false;
  }
  setCell(move.from, CellState::Empty);
  setCell(move.to, static_cast<CellState>(fromState));
  return true;
}

std::vector<Position> Board::getPiecesForPlayer(int player) const {
  std::vector<Position> pieces;
  for (int x = 0; x <= BOARD_SIZE; ++x) {
    for (int y = 0; y <= BOARD_SIZE; ++y) {
      if (grid_[x][y] == static_cast<CellState>(player)) {
        pieces.push_back(getCoordinates({x, y}));
      }
    }
  }
  return pieces;
}

void Board::reset(const std::vector<int> &players) {
  players_ = players;
  initializeBoard();
}

// Move generation methods moved from Player
std::list<Move> Board::getAllAvailableMovesForPlayer(int playerId) const {
  std::list<Move> validMoves;
  for (int x = -BOARD_SIZE / 2; x <= BOARD_SIZE / 2; ++x) {
    for (int y = -BOARD_SIZE / 2; y <= BOARD_SIZE / 2; ++y) {
      Position pos = {x, y};
      if (getCell(pos) == static_cast<CellState>(playerId)) {
        auto moves = getAvailableMoves(playerId, pos);
        validMoves.insert(validMoves.end(), moves.begin(), moves.end());
      }
    }
  }
  return validMoves;
}

std::list<Move> Board::getAvailableMoves(int playerId,
                                         const Position &pos) const {
  std::list<Move> moves = getWalkMoves(playerId, pos);
  std::list<Move> hopMoves = getHopMoves(playerId, pos, {});
  moves.insert(moves.end(), hopMoves.begin(), hopMoves.end());
  // Filtering by region if needed can be done here
  return moves;
}

std::list<Move> Board::getWalkMoves(int playerId, const Position &pos) const {
  std::list<Move> moves;
  for (const auto &dir : ValidDirections) {
    Position newPos = {pos.x + dir.x, pos.y + dir.y};
    if (isValidPosition(newPos) && getCell(newPos) == CellState::Empty) {
      moves.push_back({pos, newPos, false, 0, {}});
    }
  }
  return moves;
}

std::list<Move>
Board::getHopMoves(int playerId, const Position &pos,
                   std::list<Position> travelledPositions) const {
  std::list<Move> moves;
  for (const auto &dir : ValidDirections) {
    Position midPos = {pos.x + dir.x, pos.y + dir.y};
    Position newPos = {pos.x + 2 * dir.x, pos.y + 2 * dir.y};
    if (isValidPosition(newPos) && getCell(midPos) != CellState::Empty &&
        getCell(midPos) != CellState::Invalid &&
        getCell(newPos) == CellState::Empty &&
        (std::find(travelledPositions.begin(), travelledPositions.end(),
                   newPos) == travelledPositions.end())) {
      moves.push_back({pos, newPos, true, 1 /* numHops*/, {}});
      travelledPositions.push_back(newPos);
      // For multi-hop, recursively find further hops
      auto furtherHops = getHopMoves(playerId, newPos, travelledPositions);
      for (const auto &hop : furtherHops) {
        Move multiHop = {pos, hop.to, true, 1 + hop.numHops, {hop.from}};
        multiHop.hopPath.insert(multiHop.hopPath.end(), hop.hopPath.begin(),
                                hop.hopPath.end());
        moves.push_back(multiHop);
      }
    }
  }
  return moves;
}

bool Board::checkWin(int playerId) const {
  GridRegion targetRegion = static_cast<GridRegion>(NUM_PLAYERS + 1 - playerId);
  // Check if all pieces of the player are in the opposite home region
  for (const auto &piece : getPiecesForPlayer(playerId)) {
    if (getRegionOfCoordinate(piece) != targetRegion) {
      return false;
    }
  }
  return true;
}

bool Board::isValidMove(const Move &move, int player) const {
  if (!isValidPosition(move.from) || !isValidPosition(move.to))
    return false;
  if (getCell(move.from) != static_cast<CellState>(player))
    return false;
  if (getCell(move.to) != CellState::Empty)
    return false;
  // TODO: Add jump logic and more rules
  return true;
}

// Player management methods
int Board::getCurrentPlayer() const {
  if (currentPlayerIdx_ < players_.size()) {
    return players_[currentPlayerIdx_];
  }
  return -1; // Invalid state
}

int Board::getCurrentPlayerIndex() const { return currentPlayerIdx_; }

void Board::setCurrentPlayer(int playerId) {
  for (size_t i = 0; i < players_.size(); ++i) {
    if (players_[i] == playerId) {
      currentPlayerIdx_ = i;
      return;
    }
  }
}

void Board::nextPlayer() {
  currentPlayerIdx_ = (currentPlayerIdx_ + 1) % players_.size();
}

const std::vector<int> &Board::getPlayers() const { return players_; }

void Board::setPlayers(const std::vector<int> &players) {
  players_ = players;
  currentPlayerIdx_ = 0;
}

} // namespace chinese_checkers
