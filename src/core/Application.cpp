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
#include <fstream>
#include <string>
#include <cstdlib>


namespace fs = std::filesystem;

namespace Tetris {

static fs::path findUiFont() {
    try {
        if (fs::exists("resources/fonts/DejaVuSans.ttf"))
            return "resources/fonts/DejaVuSans.ttf";
        if (fs::exists("./resources/fonts/DejaVuSans.ttf"))
            return "./resources/fonts/DejaVuSans.ttf";
        if (fs::exists("../resources/fonts/DejaVuSans.ttf"))
            return "../resources/fonts/DejaVuSans.ttf";
    } catch (...) {
    }
    return {};
}

// --- Slider helpers -------------------------------------------------------

static float clamp01(float v) {
    if (v < 0.f) return 0.f;
    if (v > 1.f) return 1.f;
    return v;
}

// Format seconds as "160 ms" etc.
static std::string formatMs(float seconds) {
    int ms = static_cast<int>(seconds * 1000.f + 0.5f);
    return std::to_string(ms) + " ms";
}

void Application::initSlider(
    Slider& slider,
    const char* label,
    float minValue,
    float maxValue,
    float* boundValue,
    float centerX,
    float y
) {
    slider.minValue   = minValue;
    slider.maxValue   = maxValue;
    slider.boundValue = boundValue;
    slider.dragging   = false;

    // Track
    const float trackW = 420.f;
    const float trackH = 4.f;

    slider.track.setSize(sf::Vector2f{trackW, trackH});
    slider.track.setOrigin(sf::Vector2f{trackW * 0.5f, trackH * 0.5f});
    slider.track.setPosition(sf::Vector2f{centerX, y});
    slider.track.setFillColor(sf::Color(80, 80, 80));

    // Knob
    const float knobW = 14.f;
    const float knobH = 28.f;
    slider.knob.setSize(sf::Vector2f{knobW, knobH});
    slider.knob.setOrigin(sf::Vector2f{knobW * 0.5f, knobH * 0.5f});
    slider.knob.setFillColor(sf::Color(230, 230, 230));
    slider.knob.setOutlineThickness(1.f);
    slider.knob.setOutlineColor(sf::Color(40, 40, 40));

    // Label (left)
    slider.label = std::make_unique<sf::Text>(m_cfgFont, label, 22);
    slider.label->setFillColor(sf::Color(220, 220, 220));
    {
        auto b = slider.label->getLocalBounds();
        const float lx = centerX - trackW * 0.5f - 10.f - b.size.x;
        const float ly = y - 18.f;
        slider.label->setPosition(sf::Vector2f{lx, ly});
    }

    // Value text (right)
    slider.valueText = std::make_unique<sf::Text>(
        m_cfgFont,
        boundValue ? formatMs(*boundValue) : std::string(""),
        20
    );
    slider.valueText->setFillColor(sf::Color(200, 200, 200));
    {
        auto b = slider.valueText->getLocalBounds();
        const float vx = centerX + trackW * 0.5f + 10.f;
        const float vy = y - 18.f;
        slider.valueText->setPosition(sf::Vector2f{vx, vy});
    }

    updateSliderVisual(slider);
}

void Application::updateSliderVisual(Slider& slider) {
    if (!slider.boundValue) return;

    const float value = *slider.boundValue;
    const float t = (slider.maxValue > slider.minValue)
                  ? clamp01((value - slider.minValue) / (slider.maxValue - slider.minValue))
                  : 0.f;

    const auto trackBounds = slider.track.getGlobalBounds();
    const float left  = trackBounds.position.x;
    const float right = trackBounds.position.x + trackBounds.size.x;
    const float y     = trackBounds.position.y;

    const float knobX = left + t * (right - left);

    slider.knob.setPosition(sf::Vector2f{knobX, y});

    if (slider.valueText) {
        slider.valueText->setString(formatMs(value));
    }
}

static const char* kConfigPath = "resources/config.json";

void Application::loadConfig() {
    // defaults first (in case file missing / broken)
    m_moveSettings.das = 0.16f;
    m_moveSettings.arr = 0.033f;

    std::ifstream in(kConfigPath, std::ios::in | std::ios::binary);
    if (!in)
        return; // no file yet, keep defaults

    std::string data(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>()
    );

    auto parseFloatField = [&](const char* key, float& out) {
        std::string pattern = std::string("\"") + key + "\"";
        std::size_t pos = data.find(pattern);
        if (pos == std::string::npos) return;

        pos = data.find(':', pos);
        if (pos == std::string::npos) return;

        const char* start = data.c_str() + pos + 1;
        char* end = nullptr;
        float v = std::strtof(start, &end);
        if (start != end) {
            out = v;
        }
    };

    parseFloatField("das", m_moveSettings.das);
    parseFloatField("arr", m_moveSettings.arr);
}

void Application::saveConfig() const {
    std::ofstream out(kConfigPath, std::ios::out | std::ios::trunc);
    if (!out)
        return; // silently fail, game still runs

    out << "{\n";
    out << "  \"das\": " << m_moveSettings.das << ",\n";
    out << "  \"arr\": " << m_moveSettings.arr << "\n";
    out << "}\n";
}

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
	loadConfig(); // <-- load DAS/ARR before using them

    // SFML 3: create window via create()
    m_window.create(sf::VideoMode({1920u, 1080u}),
                "Tetris SRS+",
                sf::State::Fullscreen);
    m_window.setFramerateLimit(240);

    m_mode         = AppMode::Title;
    m_selectedMenu = MenuItem::Sprint;

    initTitleSprites();   // title textures + sprites + layout

	initConfigUi();

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
			saveConfig();
            m_window.close();
            continue;
        }

        // ---- KeyPressed ----
        if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
            using K = sf::Keyboard::Scancode;

            // -------- TITLE SCREEN INPUT --------
            if (m_mode == AppMode::Title) {
                switch (kp->scancode) {
                    case K::Up: {
                        switch (m_selectedMenu) {
                            case MenuItem::Sprint:  m_selectedMenu = MenuItem::Config;  break;
                            case MenuItem::Endless: m_selectedMenu = MenuItem::Sprint;  break;
                            case MenuItem::Blitz:   m_selectedMenu = MenuItem::Endless; break;
                            case MenuItem::Config:  m_selectedMenu = MenuItem::Blitz;   break;
                        }
                        updateMenuHighlight();
                    } break;

                    case K::Down: {
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
    					if (m_selectedMenu == MenuItem::Config) {
    					    // go to config screen instead of starting the game
    					    m_mode = AppMode::Config;
    					} else {
    					    startGame();
    					}
    					return;

                    case K::Escape:
                        m_window.close();
                        return;

                    default:
                        break;
                }

                // handled entirely on title
                continue;
            }

			// -------- CONFIG SCREEN INPUT --------
			if (m_mode == AppMode::Config) {
			    switch (kp->scancode) {
			        case K::Escape:
			            // back to title
						saveConfig();
			            m_mode = AppMode::Title;
			            return;

			        case K::Enter:
			        case K::Space:
			            // also go back for now
						saveConfig();
			            m_mode = AppMode::Title;
			            return;

			        default:
			            break;
			    }

			    // handled in config; don't fall through to gameplay
			    continue;
			}

            // -------- GAMEPLAY INPUT --------
            switch (kp->scancode) {
                case K::Left: {
                    // one immediate step
                    if (tryMove(m_state, -1, 0)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }

                    // start DAS for left, cancel right
                    m_leftState.held      = true;
                    m_leftState.heldTime  = 0.f;
                    m_rightState.held     = false;
                    m_rightState.heldTime = 0.f;
                } break;

                case K::Right: {
                    // one immediate step
                    if (tryMove(m_state,  1, 0)
                        && m_state.grounded
                        && m_state.lockResets < m_state.maxLockResets) {
                        m_state.lockTimer = 0.f;
                        ++m_state.lockResets;
                    }

                    // start DAS for right, cancel left
                    m_rightState.held      = true;
                    m_rightState.heldTime  = 0.f;
                    m_leftState.held       = false;
                    m_leftState.heldTime   = 0.f;
                } break;

                case K::Down: {
                    tryMove(m_state, 0, -1); // soft drop
                } break;

                case K::Space: { // hard drop
                    m_state.active = dropToGround(m_state);
                    lockPiece(m_state);
                    clearLines(m_state);
                    spawn(m_state);
                    m_state.canHold = true;
                } break;

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

                // HOLD key (missing case you mentioned)
                case K::C: {
    			if (!m_state.canHold)
    			    break;

    			if (!m_state.hasHold) {
    			    // first time holding: move current piece into hold and spawn a new one
    			    m_state.hasHold  = true;
    			    m_state.holdType = m_state.active.type;
    			    spawn(m_state);             // standard spawn from bag
    			} else {
    			    // swap current piece with hold
        			Tetromino oldHold = m_state.holdType;
        			m_state.holdType  = m_state.active.type;

			        // reuse spawn() to reset position/orientation,
			        // then override the type with the held one
			        spawn(m_state);
			        m_state.active.type = oldHold;
			    }

			    m_state.canHold = false;
			} break;


                case K::Escape:
			            // back to title
			            m_mode = AppMode::Title;
			            return;

                default:
                    break;
            }
        }

        // ---- KeyReleased: stop DAS when letting go ----
        if (const auto* kr = ev->getIf<sf::Event::KeyReleased>()) {
            using K = sf::Keyboard::Scancode;
            switch (kr->scancode) {
                case K::Left:
                    m_leftState.held     = false;
                    m_leftState.heldTime = 0.f;
                    break;
                case K::Right:
                    m_rightState.held     = false;
                    m_rightState.heldTime = 0.f;
                    break;
                default:
                    break;
            }
        }

		// CONFIG mouse input: sliders
        if (m_mode == AppMode::Config) {
            if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Left) {
                    const sf::Vector2f pos(
                        static_cast<float>(mb->position.x),
                        static_cast<float>(mb->position.y)
                    );
                    onConfigMousePressed(pos);
                }
            }

            if (const auto* mr = ev->getIf<sf::Event::MouseButtonReleased>()) {
                if (mr->button == sf::Mouse::Button::Left) {
                    onConfigMouseReleased();
                }
            }

            if (const auto* mm = ev->getIf<sf::Event::MouseMoved>()) {
                const sf::Vector2f pos(
                    static_cast<float>(mm->position.x),
                    static_cast<float>(mm->position.y)
                );
                onConfigMouseMoved(pos);
            }
        }
    }
}

void Application::onConfigMousePressed(const sf::Vector2f& mousePos) {
    if (!m_cfgFontOk) return;

    auto checkSlider = [&](Slider& s) {
        auto knobBounds  = s.knob.getGlobalBounds();
        auto trackBounds = s.track.getGlobalBounds();

        if (knobBounds.contains(mousePos) || trackBounds.contains(mousePos)) {
            s.dragging = true;

            // project mouse X to [0,1] along track
            float left  = trackBounds.position.x;
            float right = trackBounds.position.x + trackBounds.size.x;
            float t = 0.f;
            if (right > left)
                t = clamp01((mousePos.x - left) / (right - left));

            if (s.boundValue) {
                *s.boundValue = s.minValue + t * (s.maxValue - s.minValue);
                updateSliderVisual(s);
            }
        }
    };

    checkSlider(m_dasSlider);
    checkSlider(m_arrSlider);
}

void Application::onConfigMouseReleased() {
    m_dasSlider.dragging = false;
    m_arrSlider.dragging = false;
}

void Application::onConfigMouseMoved(const sf::Vector2f& mousePos) {
    auto updateDraggingSlider = [&](Slider& s) {
        if (!s.dragging || !s.boundValue) return;

        auto trackBounds = s.track.getGlobalBounds();
        float left  = trackBounds.position.x;
        float right = trackBounds.position.x + trackBounds.size.x;
        if (right <= left) return;

        float t = clamp01((mousePos.x - left) / (right - left));
        *s.boundValue = s.minValue + t * (s.maxValue - s.minValue);
        updateSliderVisual(s);
    };

    updateDraggingSlider(m_dasSlider);
    updateDraggingSlider(m_arrSlider);
}

void Application::updateAutoShift(float dt) {
    auto stepSide = [&](MoveKeyState& st, int dir) {
        if (!st.held)
            return;

        st.heldTime += dt;

        // not past DAS yet → no auto-repeat
        if (st.heldTime < m_moveSettings.das)
            return;

        float extra = st.heldTime - m_moveSettings.das;
        float arr   = (m_moveSettings.arr <= 0.f) ? 0.f : m_moveSettings.arr;

        bool moved = false;

        if (arr == 0.f) {
            // ARR = 0 → move every frame after DAS
            moved = tryMove(m_state, dir, 0);
        } else {
            while (extra >= 0.f) {
                if (tryMove(m_state, dir, 0))
                    moved = true;
                extra -= arr;
            }
        }

        if (moved
            && m_state.grounded
            && m_state.lockResets < m_state.maxLockResets) {
            m_state.lockTimer = 0.f;
            ++m_state.lockResets;
        }

        // keep timer bounded
        if (arr > 0.f)
            st.heldTime = m_moveSettings.das + std::max(extra, 0.f);
    };

    // left and right
    stepSide(m_leftState,  -1);
    stepSide(m_rightState, +1);
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

void Application::initConfigUi() {
    const auto fontPath = findUiFont();
    if (fontPath.empty() || !m_cfgFont.openFromFile(fontPath.string())) {
        m_cfgFontOk = false;
        m_cfgTitle.reset();
        m_cfgBody.reset();
        m_cfgHint.reset();
        return;
    }

    m_cfgFontOk = true;

    const auto winSize = m_window.getSize();
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    // Title
    m_cfgTitle = std::make_unique<sf::Text>(m_cfgFont, "CONFIG", 40);
    m_cfgTitle->setFillColor(sf::Color(230, 230, 230));
    {
        auto b = m_cfgTitle->getLocalBounds();
        const float x = (winW - b.size.x) * 0.5f;
        const float y = winH * 0.14f;
        m_cfgTitle->setPosition(sf::Vector2f{x, y});
    }

    // Subtitle / body
    m_cfgBody = std::make_unique<sf::Text>(
        m_cfgFont,
        "Movement settings (DAS / ARR)",
        22
    );
    m_cfgBody->setFillColor(sf::Color(200, 200, 200));
    {
        auto b = m_cfgBody->getLocalBounds();
        const float x = (winW - b.size.x) * 0.5f;
        const float y = winH * 0.24f;
        m_cfgBody->setPosition(sf::Vector2f{x, y});
    }

    // Hint
    m_cfgHint = std::make_unique<sf::Text>(
        m_cfgFont,
        "Esc: Back to title",
        18
    );
    m_cfgHint->setFillColor(sf::Color(150, 150, 150));
    {
        auto b = m_cfgHint->getLocalBounds();
        const float x = (winW - b.size.x) * 0.5f;
        const float y = winH * 0.86f;
        m_cfgHint->setPosition(sf::Vector2f{x, y});
    }

    const float centerX = winW * 0.5f;
    const float dasY    = winH * 0.40f;
    const float arrY    = winH * 0.52f;

    // Common ranges (tweak to taste)
    // DAS: 40 ms .. 300 ms
    // ARR: 0 ms .. 80 ms
    initSlider(m_dasSlider, "DAS", 0.040f, 0.300f, &m_moveSettings.das, centerX, dasY);
    initSlider(m_arrSlider, "ARR", 0.000f, 0.080f, &m_moveSettings.arr, centerX, arrY);
}

void Application::startGame() {
    m_mode  = AppMode::Playing;
    m_state = GameState{};
    spawn(m_state);
}

void Application::update(float dt) {
    m_hud.update(dt);

    if (m_mode == AppMode::Title || m_mode == AppMode::Config) {
        return; // no gravity / lock while not playing
    }

	// auto-shift for held left/right
    updateAutoShift(dt);

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

void Application::renderConfig() {
    // Background panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f{
        static_cast<float>(m_window.getSize().x),
        static_cast<float>(m_window.getSize().y)
    });
    panel.setFillColor(sf::Color(8, 10, 16));
    m_window.draw(panel);

    if (!m_cfgFontOk)
        return;

    if (m_cfgTitle) m_window.draw(*m_cfgTitle);
    if (m_cfgBody)  m_window.draw(*m_cfgBody);
    if (m_cfgHint)  m_window.draw(*m_cfgHint);

    // DAS slider
    m_window.draw(m_dasSlider.track);
    m_window.draw(m_dasSlider.knob);
    if (m_dasSlider.label)     m_window.draw(*m_dasSlider.label);
    if (m_dasSlider.valueText) m_window.draw(*m_dasSlider.valueText);

    // ARR slider
    m_window.draw(m_arrSlider.track);
    m_window.draw(m_arrSlider.knob);
    if (m_arrSlider.label)     m_window.draw(*m_arrSlider.label);
    if (m_arrSlider.valueText) m_window.draw(*m_arrSlider.valueText);
}

void Application::render() {
    m_window.clear(Colors::Bg);

    if (m_mode == AppMode::Title) {
        // Title uses screen-space coordinates
        m_window.setView(m_window.getDefaultView());
        renderTitle();
    } else if (m_mode == AppMode::Config) {
        renderConfig();
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
