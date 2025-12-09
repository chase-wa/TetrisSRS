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

inline int clearLines(GameState& s) {
    int writeRow = 0;
    int cleared  = 0;

    // scan from bottom (y=0) to top (y=ROWS-1)
    for (int y = 0; y < ROWS; ++y) {
        bool full = true;
        for (int x = 0; x < COLS; ++x) {
            if (s.grid[y * COLS + x] == 0) {
                full = false;
                break;
            }
        }

        if (!full) {
            // keep this row; move it down if needed
            if (writeRow != y) {
                for (int x = 0; x < COLS; ++x) {
                    s.grid[writeRow * COLS + x] = s.grid[y * COLS + x];
                }
            }
            ++writeRow;
        } else {
            // drop this row
            ++cleared;
        }
    }

    // clear remaining rows at the top
    for (int y = writeRow; y < ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            s.grid[y * COLS + x] = 0;
        }
    }

    return cleared;
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
    p.x    = 4;                          // your spawn X; change if you like
    p.y    = spawnYVisible(t, 0);        // spawn so top of piece is visible
    return p;
}

// spawn from bag or forced type (used by hold)
inline void spawn(GameState& s, std::optional<Tetromino> forced = std::nullopt) {
    const Tetromino t = forced ? *forced : s.bag.next();

    s.active = makeSpawnPiece(t);

    // reset dynamic state
    s.lockTimer  = 0.f;
    s.lockResets = 0;
    s.grounded   = false;
    s.fallAcc    = 0.f;
    s.canHold    = true;

    // If the top is clogged, slide down until not blocked (simple fail-soft).
    while (blocked(s, s.active)) {
        s.active.y -= 1;                   // move downward (toward y=0)
        if (s.active.y < 0) {
            s.active.y = 0;
            break;                         // you can trigger game over here instead
        }
    }
}

// hold / swap current piece
inline void holdPiece(GameState& s) {
    if (!s.canHold)
        return;

    s.canHold = false;

    if (!s.hasHold) {
        s.holdType = s.active.type;
        s.hasHold  = true;
        spawn(s);            // but this no longer flips canHold back
    } else {
        Tetromino current = s.active.type;
        Tetromino held    = s.holdType;
        s.holdType        = current;
        spawn(s, held);
    }
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
