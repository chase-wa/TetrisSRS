#pragma once
#include <array>
#include <cstdint>

namespace Tetris {

constexpr int COLS = 10;
constexpr int ROWS = 22; // 20 visible + 2 buffer
constexpr int VISIBLE_ROWS = 20;
constexpr unsigned CELL = 32; // px

using Cell = std::uint8_t;

struct GameState {
    std::array<Cell, COLS * ROWS> grid{}; // 0 empty
};

} // namespace Tetris
