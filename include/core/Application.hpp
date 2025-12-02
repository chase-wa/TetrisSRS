#pragma once
#include <SFML/Graphics/RenderWindow.hpp>
#include "game/GameState.hpp"
#include "render/PlayfieldRenderer.hpp"
#include "render/Hud.hpp"

namespace Tetris {

    class Application {
    public:
        Application();
        void run();

    private:
        void processEvents();
        void update(float dt);
        void render();

        sf::RenderWindow m_window;
        GameState m_state;
        PlayfieldRenderer m_playfield;
        Hud m_hud;
    };

} // namespace Tetris
