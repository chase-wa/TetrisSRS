// Application.hpp
#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <memory>

#include "game/GameState.hpp"
#include "render/PlayfieldRenderer.hpp"
#include "render/Hud.hpp"

namespace Tetris {

struct MoveSettings {
    float das = 0.16f;   // seconds
    float arr = 0.033f;  // seconds
};

struct MoveKeyState {
    bool  held      = false;
    float heldTime  = 0.f; // total time key has been held
};

enum class AppMode {
    Title,
    Playing,
    Config
};

enum class MenuItem {
    Sprint,
    Endless,
    Blitz,
    Config
};

// Simple UI slider used on the config screen
struct Slider {
    sf::RectangleShape track;
    sf::RectangleShape knob;
    std::unique_ptr<sf::Text> label;
    std::unique_ptr<sf::Text> valueText;

    float minValue   = 0.f;
    float maxValue   = 1.f;
    float* boundValue = nullptr;   // points into MoveSettings
    bool  dragging   = false;
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

    void updateAutoShift(float dt);

    // --- config screen UI ---
    void initConfigUi();
    void renderConfig();

    // slider helpers (config screen)
    void initSlider(
        Slider& slider,
        const char* label,
        float minValue,
        float maxValue,
        float* boundValue,
        float centerX,
        float y
    );
    void updateSliderVisual(Slider& slider);
    void onConfigMousePressed(const sf::Vector2f& mousePos);
    void onConfigMouseReleased();
    void onConfigMouseMoved(const sf::Vector2f& mousePos);

    void loadConfig();
    void saveConfig() const;

    sf::RenderWindow  m_window;
    GameState         m_state;
    PlayfieldRenderer m_playfield;
    Hud               m_hud;

    AppMode  m_mode;
    MenuItem m_selectedMenu;

    MoveSettings m_moveSettings;
    MoveKeyState m_leftState;
    MoveKeyState m_rightState;

    // config text
    sf::Font m_cfgFont;
    bool m_cfgFontOk = false;
    std::unique_ptr<sf::Text> m_cfgTitle;
    std::unique_ptr<sf::Text> m_cfgBody;
    std::unique_ptr<sf::Text> m_cfgHint;

    // config sliders
    Slider m_dasSlider;
    Slider m_arrSlider;

    // title screen textures
    sf::Texture m_titleBgTex;
    sf::Texture m_homeBarTex;
    sf::Texture m_sprintTex;
    sf::Texture m_endlessTex;
    sf::Texture m_blitzTex;
    sf::Texture m_configTex;

    // title screen sprites
    std::unique_ptr<sf::Sprite> m_titleBgSprite;
    std::unique_ptr<sf::Sprite> m_homeBarSprite;
    std::unique_ptr<sf::Sprite> m_sprintSprite;
    std::unique_ptr<sf::Sprite> m_endlessSprite;
    std::unique_ptr<sf::Sprite> m_blitzSprite;
    std::unique_ptr<sf::Sprite> m_configSprite;

    sf::RectangleShape m_menuHighlight;
};

} // namespace Tetris
