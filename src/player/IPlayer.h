#pragma once
#include "../Board.h"
#include <list>

namespace chinese_checkers {

class IPlayer {
public:
  virtual Move getNextMove() const = 0;
  virtual ~IPlayer() = default;
};

} // namespace chinese_checkers
