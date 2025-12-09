#pragma once

namespace Tetris {

// dx: right+, dy: up+
struct Kick { int dx; int dy; };

// 180° mode flag (you can ignore Basic for now if you want)
enum class Kick180Mode { SRS_180_BASIC, SRSX_180 };

// Simple view over a kick list
struct KickList {
    const Kick* data;
    int count;
};

// --- JLSTZ 90° kicks (ported from WALL_KICKS, dy flipped for y-up) ---
inline KickList getWallKicksJLSTZ(int fromRot, int toRot) {
    // keys: (0,1),(1,0),(1,2),(2,1),(2,3),(3,2),(3,0),(0,3)
    // Python coords use y-down; here dy = -dy_py.

    static const Kick K_0_1[] = {     // (0,1): [(0,0),(-1,0),(-1,1),(0,-2),(-1,-2)]
        { 0,  0}, { -1,  0}, { -1, -1}, {  0,  2}, { -1,  2}
    };
    static const Kick K_1_0[] = {     // (1,0): [(0,0),(1,0),(1,-1),(0,2),(1,2)]
        { 0,  0}, {  1,  0}, {  1,  1}, {  0, -2}, {  1, -2}
    };
    static const Kick K_1_2[] = {     // (1,2): same as (1,0)
        { 0,  0}, {  1,  0}, {  1,  1}, {  0, -2}, {  1, -2}
    };
    static const Kick K_2_1[] = {     // (2,1): [(0,0),(-1,0),(-1,1),(0,-2),(-1,-2)]
        { 0,  0}, { -1,  0}, { -1, -1}, {  0,  2}, { -1,  2}
    };
    static const Kick K_2_3[] = {     // (2,3): [(0,0),(1,0),(1,1),(0,-2),(1,-2)]
        { 0,  0}, {  1,  0}, {  1, -1}, {  0,  2}, {  1,  2}
    };
    static const Kick K_3_2[] = {     // (3,2): [(0,0),(-1,0),(-1,-1),(0,2),(-1,2)]
        { 0,  0}, { -1,  0}, { -1,  1}, {  0, -2}, { -1, -2}
    };
    static const Kick K_3_0[] = {     // (3,0): same as (3,2)
        { 0,  0}, { -1,  0}, { -1,  1}, {  0, -2}, { -1, -2}
    };
    static const Kick K_0_3[] = {     // (0,3): [(0,0),(1,0),(1,1),(0,-2),(1,-2)]
        { 0,  0}, {  1,  0}, {  1, -1}, {  0,  2}, {  1,  2}
    };

    static const Kick DEFAULT[] = { {0,0} };

    if (fromRot == 0 && toRot == 1) return {K_0_1, 5};
    if (fromRot == 1 && toRot == 0) return {K_1_0, 5};
    if (fromRot == 1 && toRot == 2) return {K_1_2, 5};
    if (fromRot == 2 && toRot == 1) return {K_2_1, 5};
    if (fromRot == 2 && toRot == 3) return {K_2_3, 5};
    if (fromRot == 3 && toRot == 2) return {K_3_2, 5};
    if (fromRot == 3 && toRot == 0) return {K_3_0, 5};
    if (fromRot == 0 && toRot == 3) return {K_0_3, 5};

    return {DEFAULT, 1};
}

// --- I-piece 90° kicks (ported from I_WALL_KICKS, dy flipped) ---
inline KickList getWallKicksI(int fromRot, int toRot) {
    static const Kick K_0_1[] = { // (0,1)
        { 0,  0}, { -2,  0}, {  1,  0}, { -2,  1}, {  1, -2}
    };
    static const Kick K_1_0[] = { // (1,0)
        { 0,  0}, {  2,  0}, { -1,  0}, {  2, -1}, { -1,  2}
    };
    static const Kick K_1_2[] = { // (1,2)
        { 0,  0}, { -1,  0}, {  2,  0}, { -1, -2}, {  2,  1}
    };
    static const Kick K_2_1[] = { // (2,1)
        { 0,  0}, {  1,  0}, { -2,  0}, {  1,  2}, { -2, -1}
    };
    static const Kick K_2_3[] = { // (2,3)
        { 0,  0}, {  2,  0}, { -1,  0}, {  2, -1}, { -1,  2}
    };
    static const Kick K_3_2[] = { // (3,2)
        { 0,  0}, { -2,  0}, {  1,  0}, { -2,  1}, {  1, -2}
    };
    static const Kick K_3_0[] = { // (3,0)
        { 0,  0}, {  1,  0}, { -2,  0}, {  1,  2}, { -2, -1}
    };
    static const Kick K_0_3[] = { // (0,3)
        { 0,  0}, { -1,  0}, {  2,  0}, { -1, -2}, {  2,  1}
    };

    static const Kick DEFAULT[] = { {0,0} };

    if (fromRot == 0 && toRot == 1) return {K_0_1, 5};
    if (fromRot == 1 && toRot == 0) return {K_1_0, 5};
    if (fromRot == 1 && toRot == 2) return {K_1_2, 5};
    if (fromRot == 2 && toRot == 1) return {K_2_1, 5};
    if (fromRot == 2 && toRot == 3) return {K_2_3, 5};
    if (fromRot == 3 && toRot == 2) return {K_3_2, 5};
    if (fromRot == 3 && toRot == 0) return {K_3_0, 5};
    if (fromRot == 0 && toRot == 3) return {K_0_3, 5};

    return {DEFAULT, 1};
}

// --- 180° kicks (ported from WALL_KICKS_180, dy flipped) ---
inline KickList getWallKicks180(bool isI, int fromRot, int toRot) {
    static const Kick K_0_2[] = { // (0,2)
        { 0,  0}, { 0, -1}, { 1,  0}, { -1,  0}, { 1, -1}, { -1, -1}
    };
    static const Kick K_2_0[] = { // (2,0)
        { 0,  0}, { 0,  1}, { -1, 0}, {  1,  0}, { -1, 1}, {  1,  1}
    };
    static const Kick K_1_3[] = { // (1,3)
        { 0,  0}, { 1,  0}, { 0, -1}, {  0, -2}, {  1, -2}, { -1, -2}
    };
    static const Kick K_3_1[] = { // (3,1)
        { 0,  0}, { -1, 0}, { 0, -1}, {  0, -2}, { -1, -2}, {  1, -2}
    };

    static const Kick DEFAULT180[] = { {0,0} };

    // For now, I uses same 180s in this port
    if (!isI) {
        if (fromRot == 0 && toRot == 2) return {K_0_2, 6};
        if (fromRot == 2 && toRot == 0) return {K_2_0, 6};
        if (fromRot == 1 && toRot == 3) return {K_1_3, 6};
        if (fromRot == 3 && toRot == 1) return {K_3_1, 6};
    } else {
        if (fromRot == 0 && toRot == 2) return {K_0_2, 6};
        if (fromRot == 2 && toRot == 0) return {K_2_0, 6};
        if (fromRot == 1 && toRot == 3) return {K_1_3, 6};
        if (fromRot == 3 && toRot == 1) return {K_3_1, 6};
    }

    return {DEFAULT180, 1};
}

} // namespace Tetris
