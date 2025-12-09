#pragma once

#include <memory>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>

namespace Tetris {

struct GameState; // from game/GameState.hpp

class Hud {
public:
    explicit Hud(sf::RenderWindow& window);

    void update(float dt);
    void draw(const GameState& state);

private:
    void drawHold(const GameState& state);
    void drawNext(const GameState& state);

    sf::RenderWindow& m_window;

    sf::Font m_font;
    bool     m_fontOk = false;

    float m_accum  = 0.f;
    int   m_frames = 0;

    // existing FPS
    std::unique_ptr<sf::Text> m_fps;

    // sprint HUD
    std::unique_ptr<sf::Text> m_linesText;   // "Lines: X"
    std::unique_ptr<sf::Text> m_sprintText;  // "SPRINT 40"
    std::unique_ptr<sf::Text> m_sprintInfo;  // time + finished
};

} // namespace Tetris
