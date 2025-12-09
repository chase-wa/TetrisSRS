#pragma once
#include "GameState.hpp"
#include "Pieces.hpp"
#include <algorithm>
#include <limits>
#include <optional>
#include <array>

namespace Tetris {

inline bool inBounds(int x, int y) {
    return x >= 0 && x < COLS && y >= 0 && y < ROWS;
}

inline bool blocked(const GameState& s, const ActivePiece& p) {
    const auto& sh = shape(p.type).cells[p.rot];
    for (const auto& c : sh) {
        const int gx = p.x + c[0];
        const int gy = p.y + c[1];
        if (!inBounds(gx, gy)) return true;
        if (s.grid[gy * COLS + gx] != 0) return true;
    }
    return false;
}

inline void lockPiece(GameState& s) {
    const auto val = cellValue(s.active.type);
    const auto& sh = shape(s.active.type).cells[s.active.rot];
    for (const auto& c : sh) {
        const int gx = s.active.x + c[0];
        const int gy = s.active.y + c[1];
        if (inBounds(gx, gy)) s.grid[gy * COLS + gx] = val;
    }
}

inline int clearLines(GameState& s)
{
    int linesCleared = 0;

    // go through each visible row
    for (int y = 0; y < ROWS; ++y) {
        bool full = true;
        for (int x = 0; x < COLS; ++x) {
            if (s.grid[y * COLS + x] == 0) {
                full = false;
                break;
            }
        }

        if (!full)
            continue;

        // shift everything above this row down by one
        for (int yy = y; yy < ROWS - 1; ++yy) {
            for (int x = 0; x < COLS; ++x) {
                s.grid[yy * COLS + x] = s.grid[(yy + 1) * COLS + x];
            }
        }

        // clear the very top row
        for (int x = 0; x < COLS; ++x) {
            s.grid[(ROWS - 1) * COLS + x] = 0;
        }

        ++linesCleared;
        --y;
    }

    if (linesCleared > 0) {
        s.totalLinesCleared += linesCleared;

        // Sprint-specific: auto-finish when we hit 40+
        if (s.runType == RunType::Sprint && s.totalLinesCleared >= 40) {
            s.gameOver = true;
        }
    }

    return linesCleared;
}


// Compute a Y so the highest block of the spawn rotation sits at the top visible row.
inline int spawnYVisible(Tetromino t, int rot = 0) {
    const auto& sh = shape(t).cells[rot];
    int maxLocalY = std::numeric_limits<int>::min();
    for (const auto& c : sh) maxLocalY = std::max(maxLocalY, static_cast<int>(c[1]));
    // top visible row is VISIBLE_ROWS - 1
    return (VISIBLE_ROWS - 1) - maxLocalY;
}

// helper: build a fresh spawn piece for a given type
inline ActivePiece makeSpawnPiece(Tetromino t) {
    ActivePiece p{};
    p.type = t;
    p.rot  = 0;
    p.x    = 3;               // your spawn X
    p.y    = VISIBLE_ROWS;    // your existing spawn Y (top of 20x10)
    return p;
}

// spawn a specific piece type without touching the bag
inline void spawnActive(GameState& s, Tetromino t) {
    s.active = makeSpawnPiece(t);

    s.grounded   = false;
    s.lockTimer  = 0.f;
    s.lockResets = 0;
}

// normal spawn from the bag / queue
inline void spawn(GameState& s) {
    Tetromino t = s.bag.next();
    spawnActive(s, t);

    // IMPORTANT: re-enable hold each time a NEW piece appears
    s.canHold = true;
}

// internal helper used by holdPiece
inline void doHold(GameState& s) {
    if (!s.canHold)
        return;

    if (!s.hasHold) {
        // first time: move current active to hold, spawn from bag
        s.holdType = s.active.type;
        s.hasHold  = true;

        spawn(s);   // uses bag, sets canHold = true (we'll immediately clear)
    } else {
        // later: swap active with held, DO NOT touch bag
        Tetromino current = s.active.type;
        Tetromino held    = s.holdType;

        s.holdType = current;    // put current into hold
        spawnActive(s, held);    // bring held piece into play
    }

    // no double-hold on the same active piece
    s.canHold = false;
}

// public API used by Application
inline void holdPiece(GameState& s) {
    doHold(s);
}


inline bool canMove(const GameState& s, const ActivePiece& p, int dx, int dy) {
    auto q = p; q.x += dx; q.y += dy;
    return !blocked(s, q);
}

// Compute where the active piece would land if dropped straight down.
inline ActivePiece dropToGround(const GameState& s) {
    ActivePiece g = s.active;
    while (true) {
        ActivePiece next = g;
        next.y -= 1;                 // down is -1 in your system
        if (blocked(s, next))        // would collide: stop
            break;
        g = next;
    }
    return g;
}

// Peek the next N tetrominoes from the bag without mutating GameState.
// This lets HUD treat the bag "like a queue" safely.
template<std::size_t N>
inline std::array<Tetromino, N> peekNextPieces(const GameState& s) {
    auto bagCopy = s.bag;                // copy the bag, not the whole state
    std::array<Tetromino, N> out{};
    for (std::size_t i = 0; i < N; ++i) {
        out[i] = bagCopy.next();
    }
    return out;
}

} // namespace Tetris
