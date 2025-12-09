#pragma once
#include <SFML/Graphics.hpp>
#include "game/GameState.hpp"

namespace Tetris {

    class PlayfieldRenderer {
    public:
        explicit PlayfieldRenderer(sf::RenderWindow& window);

        void setOriginPx(sf::Vector2f origin) { m_origin = origin; }
        void draw(const GameState& gs);

    private:
        void drawGrid();
        void drawCells(const GameState& gs);
		void drawGhost(const GameState& gs);
        void drawActive(const GameState& gs);

        sf::RenderWindow& m_window;
        sf::Vector2f m_origin{64.f, 64.f}; // left/top of playfield in pixels
    };

} // namespace Tetris
