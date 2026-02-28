/*

    Chinese Checkers Hex-Star Board (schematic):

------------*------------
-----------*-*-----------
----------*-*-*----------
---------*-*-*-*---------
*-*-*-*-*-*-*-*-*-*-*-*-*
-*-*-*-*-*-*-*-*-*-*-*-*-
--*-*-*-*-*-*-*-*-*-*-*--
---*-*-*-*-*-*-*-*-*-*---
----*-*-*-*-*-*-*-*-*----
---*-*-*-*-*-*-*-*-*-*---
--*-*-*-*-*-*-*-*-*-*-*--`
-*-*-*-*-*-*-*-*-*-*-*-*-
*-*-*-*-*-*-*-*-*-*-*-*-*
---------*-*-*-*---------
----------*-*-*----------
-----------*-*-----------
------------*------------

Each '*' represents a valid cell in the star grid while the '-' represents an
invalid cell.

*/

#ifndef CHINESE_CHECKER_BOARD_H
#define CHINESE_CHECKER_BOARD_H

#include <array>
#include <cstddef>
#include <iostream>
#include <list>
#include <vector>

namespace chinese_checkers {

constexpr int BOARD_SIZE = 24; // Board size set 24 x 24
constexpr int NUM_PLAYERS = 6;
constexpr int PIECES_PER_PLAYER = 10;

// Board cell states
enum class CellState {
  Invalid = 0, // Not part of the board
  Player1 = 1, // Occupied by Player 1 at the start
  Player2 = 2, // Occupied by Player 2 at the start
  Player3 = 3, // Occupied by Player 3 at the start
  Player4 = 4, // Occupied by Player 4 at the start
  Player5 = 5, // Occupied by Player 5 at the start
  Player6 = 6, // Occupied by Player 6 at the start
  Empty = 7,   // Part of the board but unoccupied
};

struct Position {
  int x;
  int y;
  bool operator==(const Position &other) const {
    return x == other.x && y == other.y;
  }
  bool operator<(const Position &other) const {
    if (x != other.x)
      return x < other.x;
    return y < other.y;
  }
};

inline const std::vector<Position> ValidDirections = {
    {1, 1}, {-1, -1}, {-1, 1}, {1, -1}, {2, 0}, {-2, 0}};

enum class GridRegion {
  Public = 0,
  Region1 = 1, // Start positions of player 1
  Region2 = 2, // Start positions of player 2
  Region3 = 3, // Start positions of player 3
  Region4 = 4, // Start positions of player 4
  Region5 = 5, // Start positions of player 5
  Region6 = 6, // Start positions of player 6
  Invalid = 7,
};

struct Move {
  Position from;
  Position to;
  bool isHop;
  int numHops;
  std::vector<Position> hopPath; // For multi-hop moves
};

static void printMove(const Move &move) {
  std::cout << "Move from (" << move.from.x << ", " << move.from.y << ") to ("
            << move.to.x << ", " << move.to.y << ")";
  if (move.isHop && move.numHops > 1) {
    std::cout << " with hops:";
    for (const auto &pos : move.hopPath) {
      std::cout << " (" << pos.x << ", " << pos.y << ")";
    }
  }
  std::cout << std::endl;
}

class Board {
public:
  Board();
  Board(std::vector<int> players);
  Board(const Board &other);            // Copy constructor
  Board &operator=(const Board &other); // Copy assignment operator
  Board clone() const;                  // Deep copy method
  CellState getCell(const Position &gridPos) const;
  void setCell(const Position &gridPos, CellState state);
  bool isValidPosition(const Position &pos) const;
  std::vector<Position> getPlayerPositions(int player) const;
  void reset(const std::vector<int> &players);
  void printGrid() const;
  bool applyMove(const Move &move);
  GridRegion getRegionOfCoordinate(const Position &coordinate) const;
  std::vector<Position> getPiecesForPlayer(int player) const;
  std::list<Move> getAllAvailableMovesForPlayer(int playerId) const;
  std::list<Move> getAvailableMoves(int playerId, const Position &pos) const;
  std::list<Move> getWalkMoves(int playerId, const Position &pos) const;
  std::list<Move> getHopMoves(int playerId, const Position &pos,
                              std::list<Position> travelledPositions) const;
  bool checkWin(int playerId) const;
  bool isValidMove(const Move &move, int player) const;

  // Add these new methods for player management
  int getCurrentPlayer() const;
  int getCurrentPlayerIndex() const;
  void setCurrentPlayer(int playerId);
  void nextPlayer();
  const std::vector<int> &getPlayers() const;
  void setPlayers(const std::vector<int> &players);

private:
  std::array<std::array<CellState, BOARD_SIZE + 1>, BOARD_SIZE + 1> grid_;
  std::vector<int> players_; // Moved from Game class
  int currentPlayerIdx_;     // Moved from Game class

  void initializeBoard();
  void initializeBoardForPlayer(int playerIdx);
  bool isInStartRegionForPlayer(int playerIdx, Position pos);
  int getDistanceToRegion(const Position &coordinate, GridRegion region) const;
  Position getCoordinates(const Position &gridPos) const {
    return {gridPos.x - BOARD_SIZE / 2, gridPos.y - BOARD_SIZE / 2};
  }
  Position getGridPosition(const Position &pos) const {
    return {pos.x + BOARD_SIZE / 2, pos.y + BOARD_SIZE / 2};
  }
};

} // namespace chinese_checkers

#endif // CHINESE_CHECKER_BOARD_H
