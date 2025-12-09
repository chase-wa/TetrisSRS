#pragma once
#include "GameState.hpp"
#include "Pieces.hpp"
#include "Kicks.hpp"
#include "Logic.hpp"

namespace Tetris {

    // drot: +1=CW, -1=CCW, ±2=180
    inline bool tryRotateWithKicks(GameState& s, int drot, Kick180Mode mode) {
        ActivePiece orig = s.active;
        const int from = orig.rot;

        int norm = drot;
        if (norm == -2) norm = 2;
        if (norm == 0) return false;

        const int to = (from + norm + 4) & 3;
        const bool isI = (orig.type == Tetromino::I);
        const bool isO = (orig.type == Tetromino::O);

        // O: no kicks, just rotate in place
        if (isO) {
            ActivePiece cand = orig;
            cand.rot = to;
            if (!blocked(s, cand)) {
                s.active = cand;
                return true;
            }
            return false;
        }

        // Choose kick list
        KickList kicks;
        if (norm == 2) {
            kicks = getWallKicks180(isI, from, to);
        } else if (isI) {
            kicks = getWallKicksI(from, to);
        } else {
            kicks = getWallKicksJLSTZ(from, to);
        }

        const int baseX = orig.x;
        const int baseY = orig.y;

        // Pass 1: like Python's piece['y'] += 1 (down) -> here y-up, so -1
        for (int pass = 0; pass < 2; ++pass) {
            const int extraDown = (pass == 0 ? -1 : 0); // -1 = one row down

            for (int i = 0; i < kicks.count; ++i) {
                ActivePiece cand = orig;
                cand.rot = to;
                cand.x = baseX + kicks.data[i].dx;
                cand.y = baseY + extraDown + kicks.data[i].dy;

                if (!blocked(s, cand)) {
                    s.active = cand;
                    return true;
                }
            }
            // second pass uses no extraDown, like the second for-loop in Python
        }

        return false;
    }

} // namespace Tetris
