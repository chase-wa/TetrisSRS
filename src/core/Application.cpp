#include "core/Application.hpp"
#include "render/Colors.hpp"
#include "render/PlayfieldRenderer.hpp"
#include "game/Logic.hpp"
#include "game/Rotate.hpp"
#include "game/Kicks.hpp"
#include "render/Hud.hpp"

#include <SFML/Window/Event.hpp>
#include <chrono>
#include <filesystem>
#include <cstdio>
#include <memory>

namespace fs = std::filesystem;

namespace Tetris {

// simple move helper
static bool tryMove(GameState& s, int dx, int dy) {
    auto p = s.active;
    p.x += dx;
    p.y += dy;
    if (!blocked(s, p)) {
        s.active = p;
        return true;
    }
    return false;
}

Application::Application()
    : m_state{}
    , m_playfield(m_window)
    , m_hud(m_window)
{
    // SFML 3: create window via create()
    m_window.create(sf::VideoMode({1920u, 1080u}),
                "Tetris SRS+",
                sf::State::Fullscreen);
    m_window.setFramerateLimit(240);

    m_mode         = AppMode::Title;
    m_selectedMenu = MenuItem::Sprint;

    initTitleSprites();   // title textures + sprites + layout

    // prepare first piece (actual start happens in startGame)
    spawn(m_state);
}

void Application::run() {
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (m_window.isOpen()) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        processEvents();
        update(dt);
        render();
    }
}

void Application::processEvents() {
    while (const auto ev = m_window.pollEvent()) {
        if (ev->is<sf::Event::Closed>()) {
            m_window.close();
            continue;
        }

        if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
            using K = sf::Keyboard::Scancode;

            // -------- TITLE SCREEN INPUT --------
            if (m_mode == AppMode::Title) {
                switch (kp->scancode) {
                    case K::Up: {
                        // cycle up through menu
                        switch (m_selectedMenu) {
                            case MenuItem::Sprint:  m_selectedMenu = MenuItem::Config;  break;
                            case MenuItem::Endless: m_selectedMenu = MenuItem::Sprint;  break;
                            case MenuItem::Blitz:   m_selectedMenu = MenuItem::Endless; break;
                            case MenuItem::Config:  m_selectedMenu = MenuItem::Blitz;   break;
                        }
                        updateMenuHighlight();
                    } break;

                    case K::Down: {
                        // cycle down through menu
                        switch (m_selectedMenu) {
                            case MenuItem::Sprint:  m_selectedMenu = MenuItem::Endless; break;
                            case MenuItem::Endless: m_selectedMenu = MenuItem::Blitz;   break;
                            case MenuItem::Blitz:   m_selectedMenu = MenuItem::Config;  break;
                            case MenuItem::Config:  m_selectedMenu = MenuItem::Sprint;  break;
                        }
                        updateMenuHighlight();
                    } break;

                    case K::Enter:
                    case K::Space:
                        startGame();
                        return; // do not process gameplay keys for this event

                    case K::Escape:
                        m_window.close();
                        return;

                    default:
                        break;
                }

                // event handled on title screen
                continue;
            }

            // -------- GAMEPLAY INPUT --------
            switch (kp->scancode) {
                case K::Left: {
                    if (tryMove(m_state, -1, 0)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }
                } break;

                case K::Right: {
                    if (tryMove(m_state,  1, 0)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }
                } break;

                case K::Down: {
                    // soft drop: one cell; no lock reset
                    tryMove(m_state, 0, -1);
                } break;

                // HARD DROP
                case K::Space: {
                    m_state.active = dropToGround(m_state);
                    lockPiece(m_state);
                    clearLines(m_state);
                    spawn(m_state);
                    m_state.canHold = true;
                } break;

                // rotations with SRS-X 180s
                case K::Up:
                case K::X: {
                    if (tryRotateWithKicks(m_state, +1, Kick180Mode::SRSX_180)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }
                } break;

                case K::Z: {
                    if (tryRotateWithKicks(m_state, -1, Kick180Mode::SRSX_180)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }
                } break;

                case K::A: { // 180°
                    if (tryRotateWithKicks(m_state, +2, Kick180Mode::SRSX_180)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }
                } break;

                    // --- HOLD ---
    			case K::C: {
        			// only allow one hold per piece
        			if (!m_state.canHold)
        			    break;

    			    if (!m_state.hasHold) {
    			        // first time: move current piece into hold, spawn new one
    			        m_state.holdType = m_state.active.type;
    			        m_state.hasHold  = true;

    			        spawn(m_state);  // uses bag.next(), resets lock/grounded/canHold
    			    } else {
    			        // swap current active with held piece
    			        std::swap(m_state.holdType, m_state.active.type);

			            m_state.active.rot = 0;
    			        m_state.active.x   = 4;
    			        m_state.active.y   = spawnYVisible(m_state.active.type, 0);

    			        m_state.lockTimer  = 0.f;
    			        m_state.lockResets = 0;
    			        m_state.grounded   = false;

    			        // fail-soft: nudge up if overlapping stack
    			        while (blocked(m_state, m_state.active)) {
    			            m_state.active.y -= 1;
    			            if (m_state.active.y < 0) {
    			                m_state.active.y = 0;
    			                break;
    			            }
    			        }
    			    }

    			    // after any hold, disable holding until this piece locks
    			    m_state.canHold = false;
    			} break;

                case K::Escape:
                    m_window.close();
                    break;

                default:
                    break;
            }
        }
    }
}

static void loadTextureOrWarn(sf::Texture& tex, const fs::path& p) {
    if (!tex.loadFromFile(p.string())) {
        std::fprintf(stderr, "[Title] Failed to load texture: %s\n",
                     p.string().c_str());
    } else {
        std::fprintf(stderr, "[Title] Loaded texture: %s\n",
                     p.string().c_str());
    }
    tex.setSmooth(true);
}

void Application::initTitleSprites() {
    fs::path base = "resources/assets/title_screen";

    if (!fs::exists(base / "HOME_BAR.png")) {
        fs::path alt = "../resources/assets/title_screen";
        if (fs::exists(alt / "HOME_BAR.png")) {
            base = alt;
        }
    }

    std::fprintf(stderr, "[Title] assets base: %s\n", base.string().c_str());

    // --- load textures ---
    loadTextureOrWarn(m_titleBgTex,  base / "BG.png");
    loadTextureOrWarn(m_homeBarTex,  base / "HOME_BAR.png");
    loadTextureOrWarn(m_sprintTex,   base / "SPRINT.png");
    loadTextureOrWarn(m_endlessTex,  base / "ENDLESS.png");
    loadTextureOrWarn(m_blitzTex,    base / "BLITZ.png");
    loadTextureOrWarn(m_configTex,   base / "CONFIG.png");

    // --- create sprites (SFML 3: must pass a texture) ---
    m_titleBgSprite  = std::make_unique<sf::Sprite>(m_titleBgTex);
    m_homeBarSprite  = std::make_unique<sf::Sprite>(m_homeBarTex);
    m_sprintSprite   = std::make_unique<sf::Sprite>(m_sprintTex);
    m_endlessSprite  = std::make_unique<sf::Sprite>(m_endlessTex);
    m_blitzSprite    = std::make_unique<sf::Sprite>(m_blitzTex);
    m_configSprite   = std::make_unique<sf::Sprite>(m_configTex);

    const auto winSize = m_window.getSize();
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    // --- background: cover window ---
    if (auto s = m_titleBgTex.getSize(); s.x > 0 && s.y > 0) {
        float scaleX = winW / static_cast<float>(s.x);
        float scaleY = winH / static_cast<float>(s.y);
        float scale  = std::max(scaleX, scaleY);
        m_titleBgSprite->setScale(sf::Vector2f{scale, scale});
        m_titleBgSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    // --- HOME bar at top, full width ---
    if (auto s = m_homeBarTex.getSize(); s.x > 0 && s.y > 0) {
        float scale = winW / static_cast<float>(s.x);
        m_homeBarSprite->setScale(sf::Vector2f{scale, scale});
        m_homeBarSprite->setPosition(sf::Vector2f{0.f, 0.f});
    }

    // --- menu bars: common scale + spacing ---
    float menuScale = 1.f;
    float rowHeight = 0.f;
    {
        auto ts = m_sprintTex.getSize();          // reference size
        if (ts.x > 0 && ts.y > 0) {
            const float targetW = winW * 0.65f;
            menuScale = targetW / static_cast<float>(ts.x);
            rowHeight = static_cast<float>(ts.y) * menuScale;
        } else {
            rowHeight = winH * 0.10f;
        }
    }

    const float startY  = winH * 0.30f;
    const float spacing = rowHeight * 1.10f;

    auto layoutBar = [&](sf::Sprite& sprite, int index) {
        const float x = winW * 0.06f;
        const float y = startY + index * spacing;
        sprite.setScale(sf::Vector2f{menuScale, menuScale});
        sprite.setPosition(sf::Vector2f{x, y});
    };

    if (m_sprintSprite)  layoutBar(*m_sprintSprite,  0);
    if (m_endlessSprite) layoutBar(*m_endlessSprite, 1);
    if (m_blitzSprite)   layoutBar(*m_blitzSprite,   2);
    if (m_configSprite)  layoutBar(*m_configSprite,  3);

    // highlight rectangle
    m_menuHighlight.setFillColor(sf::Color(255, 255, 255, 28));
    m_menuHighlight.setOutlineColor(sf::Color(255, 255, 255, 120));
    m_menuHighlight.setOutlineThickness(2.f);

    updateMenuHighlight();
}

sf::Sprite& Application::spriteForMenu(MenuItem item) {
    switch (item) {
        case MenuItem::Sprint:   return *m_sprintSprite;
        case MenuItem::Endless:  return *m_endlessSprite;
        case MenuItem::Blitz:    return *m_blitzSprite;
        case MenuItem::Config:   return *m_configSprite;
    }
    // fallback
    return *m_sprintSprite;
}

void Application::updateMenuHighlight() {
    if (!m_sprintSprite) return; // nothing loaded yet

    sf::Sprite& spr = spriteForMenu(m_selectedMenu);
    auto bounds = spr.getGlobalBounds(); // has position/size in SFML 3

    const float pad = 8.f;
    m_menuHighlight.setSize(sf::Vector2f{
        bounds.size.x + pad * 2.f,
        bounds.size.y + pad * 2.f
    });
    m_menuHighlight.setPosition(sf::Vector2f{
        bounds.position.x - pad,
        bounds.position.y - pad
    });
}

void Application::startGame() {
    m_mode  = AppMode::Playing;
    m_state = GameState{};
    spawn(m_state);
}

void Application::update(float dt) {
    m_hud.update(dt);

    if (m_mode == AppMode::Title) {
        return;
    }

    m_state.fallAcc += m_state.gravity * dt;
    bool movedDown = false;

    while (m_state.fallAcc >= 1.f) {
        m_state.fallAcc -= 1.f;
        if (tryMove(m_state, 0, -1)) {
            movedDown = true;
        } else {
            m_state.grounded = true;
            break;
        }
    }

    if (m_state.grounded) {
        m_state.lockTimer += dt;
        if (canMove(m_state, m_state.active, 0, -1)) {
            m_state.grounded    = false;
            m_state.lockTimer   = 0.f;
            m_state.lockResets  = 0;
        } else if (m_state.lockTimer >= m_state.lockDelay) {
            lockPiece(m_state);
            clearLines(m_state);
            spawn(m_state);
        }
    } else if (movedDown) {
        m_state.grounded = canMove(m_state, m_state.active, 0, -1)
                         ? false
                         : m_state.grounded;
        if (!m_state.grounded) {
            m_state.lockTimer  = 0.f;
            m_state.lockResets = 0;
        }
    }
}

void Application::renderTitle() {
    if (m_titleBgSprite)   m_window.draw(*m_titleBgSprite);
    if (m_homeBarSprite)   m_window.draw(*m_homeBarSprite);

    m_window.draw(m_menuHighlight);

    if (m_sprintSprite)    m_window.draw(*m_sprintSprite);
    if (m_endlessSprite)   m_window.draw(*m_endlessSprite);
    if (m_blitzSprite)     m_window.draw(*m_blitzSprite);
    if (m_configSprite)    m_window.draw(*m_configSprite);
}


void Application::render() {
    m_window.clear(Colors::Bg);

    if (m_mode == AppMode::Title) {
        // Title uses screen-space coordinates
        m_window.setView(m_window.getDefaultView());
        renderTitle();
    } else {
        // Playfield (uses whatever view PlayfieldRenderer wants)
        m_playfield.draw(m_state);

        // Reset to default view for HUD (screen-space)
        m_window.setView(m_window.getDefaultView());
        m_hud.draw(m_state);
    }

    m_window.display();
}

} // namespace Tetris
