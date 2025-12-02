#include "core/Application.hpp"
#include "render/Colors.hpp"
#include <SFML/Window/Event.hpp>
#include <chrono>

namespace Tetris {

    Application::Application()
    : m_window(sf::VideoMode({1920u, 1080u}), "Tetris SRS+", sf::State::Windowed)
    , m_playfield(m_window)
    , m_hud(m_window)
    {
        m_window.setVerticalSyncEnabled(true);
        // seed a few test cells
        m_state.grid[(ROWS-1)*COLS + 4] = 1;
        m_state.grid[(ROWS-2)*COLS + 4] = 1;
        m_state.grid[(ROWS-2)*COLS + 5] = 1;
        // center playfield horizontally
        const float totalW = COLS * CELL;
        const float x = (m_window.getSize().x - totalW) * 0.5f;
        m_playfield.setOriginPx({x, 32.f});
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
            } else if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                if (kp->scancode == sf::Keyboard::Scancode::Escape) {
                    m_window.close();
                }
            }
        }
    }


    void Application::update(float dt) {
        m_hud.update(dt);
    }

    void Application::render() {
        m_window.clear(Colors::Bg);
        m_playfield.draw(m_state);
        m_hud.draw();
        m_window.display();
    }

} // namespace Tetris
