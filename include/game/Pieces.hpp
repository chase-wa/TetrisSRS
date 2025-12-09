#pragma once
#include <array>
#include <cstdint>
#include <SFML/Graphics/Color.hpp>

namespace Tetris {

enum class Tetromino : uint8_t { I, J, L, O, S, T, Z };

struct KicklessShape {
    // 4 rotations, each is 4 blocks (x,y)
    std::array<std::array<std::array<int8_t, 2>, 4>, 4> cells;
};

// Spawn orientations (SRS-ish, no kicks yet)
inline const KicklessShape& shape(Tetromino t) {
    static const KicklessShape I{{{
        {{ {-1,0},{0,0},{1,0},{2,0} }},     // 0°
        {{ {1,1},{1,0},{1,-1},{1,-2} }},    // 90°
        {{ {-1,-1},{0,-1},{1,-1},{2,-1} }}, // 180°
        {{ {0,1},{0,0},{0,-1},{0,-2} }}     // 270°
    }}};
    static const KicklessShape J{{{
        {{ {-1,0},{0,0},{1,0},{-1,1} }},
        {{ {0,1},{0,0},{0,-1},{1,1} }},
        {{ {-1,0},{0,0},{1,0},{1,-1} }},
        {{ {0,1},{0,0},{0,-1},{-1,-1} }}
    }}};
    static const KicklessShape L{{{
        {{ {-1,0},{0,0},{1,0},{1,1} }},
        {{ {0,1},{0,0},{0,-1},{1,-1} }},
        {{ {-1,0},{0,0},{1,0},{-1,-1} }},
        {{ {0,1},{0,0},{0,-1},{-1,1} }}
    }}};
    static const KicklessShape O{{{
        {{ {0,0},{1,0},{0,1},{1,1} }},
        {{ {0,0},{1,0},{0,1},{1,1} }},
        {{ {0,0},{1,0},{0,1},{1,1} }},
        {{ {0,0},{1,0},{0,1},{1,1} }}
    }}};
    static const KicklessShape S{{{
        {{ {-1,0},{0,0},{0,1},{1,1} }},
        {{ {0,1},{0,0},{1,0},{1,-1} }},
        {{ {-1,-1},{0,-1},{0,0},{1,0} }},
        {{ {-1,1},{-1,0},{0,0},{0,-1} }}
    }}};
    static const KicklessShape T{{{
        {{ {-1,0},{0,0},{1,0},{0,1} }},
        {{ {0,1},{0,0},{0,-1},{1,0} }},
        {{ {-1,0},{0,0},{1,0},{0,-1} }},
        {{ {0,1},{0,0},{0,-1},{-1,0} }}
    }}};
    static const KicklessShape Z{{{
        {{ {-1,1},{0,1},{0,0},{1,0} }},
        {{ {1,1},{1,0},{0,0},{0,-1} }},
        {{ {-1,0},{0,0},{0,-1},{1,-1} }},
        {{ {0,1},{0,0},{-1,0},{-1,-1} }}
    }}};
    switch (t) {
        case Tetromino::I: return I; case Tetromino::J: return J; case Tetromino::L: return L;
        case Tetromino::O: return O; case Tetromino::S: return S; case Tetromino::T: return T; default: return Z;
    }
}

inline uint8_t cellValue(Tetromino t) { return static_cast<uint8_t>(t) + 1; }

inline sf::Color color(Tetromino t) {
    switch (t) {
        case Tetromino::I: return {  0,255,255};
        case Tetromino::J: return {  0,  0,255};
        case Tetromino::L: return {255,165,  0};
        case Tetromino::O: return {255,255,  0};
        case Tetromino::S: return {  0,255,  0};
        case Tetromino::T: return {160, 32,240};
        case Tetromino::Z: return {255,  0,  0};
    }
    return {180,180,180};
}

inline sf::Color colorFromCell(uint8_t v) {
    if (v == 0) return sf::Color(0,0,0,0);
    return color(static_cast<Tetromino>(v - 1));
}

} // namespace Tetris
