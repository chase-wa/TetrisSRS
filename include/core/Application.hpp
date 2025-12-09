// Application.hpp
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

#include "game/GameState.hpp"
#include "render/PlayfieldRenderer.hpp"
#include "render/Hud.hpp"

namespace Tetris {

enum class AppMode {
    Title,
    Playing
};

enum class MenuItem {
    Sprint,
    Endless,
    Blitz,
    Config
};

class Application {
public:
    Application();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();

    void renderTitle();

    void initTitleSprites();
    void updateMenuHighlight();
    sf::Sprite& spriteForMenu(MenuItem item);

    void startGame();

    sf::RenderWindow m_window;
    GameState        m_state;
    PlayfieldRenderer m_playfield;
    Hud               m_hud;

    AppMode  m_mode;
    MenuItem m_selectedMenu;

    // title screen textures
    sf::Texture m_titleBgTex;
    sf::Texture m_homeBarTex;
    sf::Texture m_sprintTex;
    sf::Texture m_endlessTex;
    sf::Texture m_blitzTex;
    sf::Texture m_configTex;

    // title screen sprites (created after textures are loaded)
    std::unique_ptr<sf::Sprite> m_titleBgSprite;
    std::unique_ptr<sf::Sprite> m_homeBarSprite;
    std::unique_ptr<sf::Sprite> m_sprintSprite;
    std::unique_ptr<sf::Sprite> m_endlessSprite;
    std::unique_ptr<sf::Sprite> m_blitzSprite;
    std::unique_ptr<sf::Sprite> m_configSprite;

    sf::RectangleShape m_menuHighlight;
};

} // namespace Tetris
