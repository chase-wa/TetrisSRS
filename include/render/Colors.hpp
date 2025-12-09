#pragma once
#include <SFML/Graphics/Color.hpp>
#include "game/Pieces.hpp"

namespace Tetris {
    namespace Colors {

        // Background colour for the window.
        inline const sf::Color Bg{16, 16, 20};

        // Grid line colour.
        inline const sf::Color Grid{42, 45, 52};


        inline const sf::Color CellEmpty{0, 0, 0, 0};

        // Colour mapping for tetromino types.
        inline sf::Color pieceColor(Tetromino t) {
            switch (t) {
                case Tetromino::I: return sf::Color(0, 255, 255);   // cyan
                case Tetromino::O: return sf::Color(255, 255,   0); // yellow
                case Tetromino::T: return sf::Color(160,   0, 240); // purple
                case Tetromino::S: return sf::Color(0, 255,   0);   // green
                case Tetromino::Z: return sf::Color(255,   0,   0); // red
                case Tetromino::J: return sf::Color(0,   0, 255);   // blue
                case Tetromino::L: return sf::Color(255, 165,   0); // orange
                default:           return sf::Color(200, 200, 200);
            }
        }

    } // namespace Colors
} // namespace Tetris
