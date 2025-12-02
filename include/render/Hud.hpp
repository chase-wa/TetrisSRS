#pragma once
#include <SFML/Graphics.hpp>

namespace Tetris {

    class Hud {
    public:
        explicit Hud(sf::RenderWindow& window);
        void update(float dt);
        void draw();

    private:
        sf::RenderWindow& m_window;
        sf::Font  m_font;
        sf::Text  m_fps;

        bool  m_fontOk = false;   // <-- add this
        float m_accum  = 0.f;
        int   m_frames = 0;
        float m_lastFps = 0.f;
    };

} // namespace Tetris
