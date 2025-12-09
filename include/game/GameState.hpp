#pragma once
#include <array>
#include <cstdint>
#include "Bag.hpp"

namespace Tetris {

constexpr int COLS = 10;
constexpr int VISIBLE_ROWS = 20;
constexpr int HIDDEN_ROWS  = 2;
constexpr int ROWS        = VISIBLE_ROWS + HIDDEN_ROWS; // 20 visible + 2 buffer
constexpr unsigned CELL   = 44; // px

using Cell = std::uint8_t;

// what kind of run is active
enum class RunType {
    Endless,
    Sprint,
    Blitz
};

struct GameState {
    std::array<Cell, COLS * ROWS> grid{};

    // falling / lock
    SevenBag   bag;
    ActivePiece active;
    float gravity    = 1.0f;  // cells per second
    float fallAcc    = 0.f;

    // lock delay
    float lockDelay  = 0.5f;  // seconds
    float lockTimer  = 0.f;
    int   lockResets = 0;
    int   maxLockResets = 15;
    bool  grounded   = false;

    // hold
    Tetromino holdType{};
    bool      hasHold   = false;
    bool      canHold   = true;

    // run / mode
    RunType runType = RunType::Endless;

    // shared scoring / state
    int  totalLinesCleared  = 0;
    bool gameOver           = false;

    // sprint-specific
    int   sprintTargetLines   = 40;
    bool  sprintCompleted     = false;
    float sprintTime          = 0.f;
    bool  sprintTimerRunning  = false;
};

} // namespace Tetris
