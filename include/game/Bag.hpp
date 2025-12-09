#pragma once
#include "Pieces.hpp"
#include "GameState.hpp"
#include <array>
#include <random>
#include <algorithm>

namespace Tetris {

class SevenBag {
public:
    SevenBag() : rng(std::random_device{}()) { refill(); }
    Tetromino next() { if (pos >= bag.size()) refill(); return bag[pos++]; }
private:
    void refill() {
        bag = {Tetromino::I, Tetromino::J, Tetromino::L, Tetromino::O, Tetromino::S, Tetromino::T, Tetromino::Z};
        std::shuffle(bag.begin(), bag.end(), rng);
        pos = 0;
    }
    std::mt19937 rng;
    std::array<Tetromino,7> bag{};
    std::size_t pos = 0;
};

struct ActivePiece {
    Tetromino type{Tetromino::T};
    int x = 3;
    int y = 19;
    int rot = 0;
};

} // namespace Tetris
