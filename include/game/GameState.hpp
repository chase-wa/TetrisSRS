#pragma once
#include <array>
#include <cstdint>
#include "Bag.hpp"

namespace Tetris {

constexpr int COLS = 10;
constexpr int VISIBLE_ROWS = 20;
constexpr int HIDDEN_ROWS  = 2;
constexpr int ROWS = VISIBLE_ROWS + HIDDEN_ROWS; // 20 visible + 2 buffer
constexpr unsigned CELL = 44; // px

using Cell = std::uint8_t;

    struct GameState {
    std::array<Cell, COLS * ROWS> grid{};

    // falling/lock
    SevenBag bag;
    ActivePiece active;
    float gravity   = 1.0f;  // cells per second
    float fallAcc   = 0.f;

    // lock delay
    float lockDelay = 0.5f;  // seconds
    float lockTimer = 0.f;
    int   lockResets = 0;
    int   maxLockResets = 15;
    bool  grounded = false;

	//hold function
	Tetromino holdType{};
    	bool      hasHold   = false;
    	bool      canHold   = true;
};

} // namespace Tetris
