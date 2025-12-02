#include "render/PlayfieldRenderer.hpp"
#include "render/Colors.hpp"

namespace Tetris {

    PlayfieldRenderer::PlayfieldRenderer(sf::RenderWindow& window)
    : m_window(window) {}

    void PlayfieldRenderer::draw(const GameState& gs) {
        drawGrid();
        drawCells(gs);
    }

    void PlayfieldRenderer::drawGrid() {
        // border
        sf::RectangleShape border;
        border.setPosition(sf::Vector2f{m_origin.x, m_origin.y});
        border.setSize(sf::Vector2f{COLS * CELL * 1.f, VISIBLE_ROWS * CELL * 1.f});
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(Colors::Grid);
        border.setOutlineThickness(2.f);
        m_window.draw(border);

        // grid lines
        sf::Vertex line[2];
        // vertical
        for (int x = 1; x < COLS; ++x) {
            float px = m_origin.x + x * CELL;
            line[0] = sf::Vertex(sf::Vector2f{px, m_origin.y}, Colors::Grid);
            line[1] = sf::Vertex(sf::Vector2f{px, m_origin.y + VISIBLE_ROWS * CELL}, Colors::Grid);
            m_window.draw(line, 2, sf::PrimitiveType::Lines);
        }
        // horizontal
        for (int y = 1; y < VISIBLE_ROWS; ++y) {
            float py = m_origin.y + y * CELL;
            line[0] = sf::Vertex(sf::Vector2f{m_origin.x, py}, Colors::Grid);
            line[1] = sf::Vertex(sf::Vector2f{m_origin.x + COLS * CELL, py}, Colors::Grid);
            m_window.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }

    void PlayfieldRenderer::drawCells(const GameState& gs) {
        sf::RectangleShape cellRect(sf::Vector2f{CELL - 2.f, CELL - 2.f});
        for (int y = 0; y < VISIBLE_ROWS; ++y) {
            for (int x = 0; x < COLS; ++x) {
                const auto v = gs.grid[(y + (ROWS - VISIBLE_ROWS)) * COLS + x];
                if (v == 0) continue;

                cellRect.setFillColor(sf::Color(180, 180, 180));
                cellRect.setPosition(sf::Vector2f{
                    m_origin.x + x * CELL + 1.f,
                    m_origin.y + (VISIBLE_ROWS - 1 - y) * CELL + 1.f
                });
                m_window.draw(cellRect);
            }
        }
    }

} // namespace Tetris
