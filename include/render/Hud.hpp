#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include "game/GameState.hpp"

namespace Tetris {

class Hud {
public:
    explicit Hud(sf::RenderWindow& window);

    void update(float dt);
    void draw(const GameState& state);

private:
    sf::RenderWindow& m_window;

    // FPS
    sf::Font                       m_font;
    bool                           m_fontOk{false};
    std::unique_ptr<sf::Text>      m_fps;       // no default ctor, so pointer
    float                          m_accum{0.f};
    int                            m_frames{0};

    // helpers
    void drawHold(const GameState& state);
    void drawNext(const GameState& state);
};

} // namespace Tetris
