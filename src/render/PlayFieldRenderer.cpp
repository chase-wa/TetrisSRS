#include "render/PlayfieldRenderer.hpp"
#include "render/Colors.hpp"
#include "game/Pieces.hpp"
#include "game/Logic.hpp"
#include "game/GameState.hpp"

namespace Tetris {

PlayfieldRenderer::PlayfieldRenderer(sf::RenderWindow& window)
    : m_window(window)
{
    m_origin = sf::Vector2f{0.f, 0.f};
}

void PlayfieldRenderer::draw(const GameState& gs) {
    const auto winSize = m_window.getSize();
    const float winW = static_cast<float>(winSize.x);
    const float winH = static_cast<float>(winSize.y);

    const float cell   = static_cast<float>(CELL);
    const float fieldW = static_cast<float>(COLS)         * cell;
    const float fieldH = static_cast<float>(VISIBLE_ROWS) * cell;

    const float verticalOffset = 0.f; // tweak if you want it higher/lower
    m_origin.x = (winW - fieldW) * 0.5f;
    m_origin.y = (winH - fieldH) * 0.5f + verticalOffset;

    drawGrid();
    drawCells(gs);
    drawGhost(gs);
    drawActive(gs);
}

void PlayfieldRenderer::drawGrid()
{
    const float cell = static_cast<float>(CELL);

    // border
    sf::RectangleShape border;
    border.setPosition(m_origin);
    border.setSize(sf::Vector2f{
        static_cast<float>(COLS) * cell,
        static_cast<float>(VISIBLE_ROWS) * cell
    });
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(Colors::Grid);
    border.setOutlineThickness(2.f);
    m_window.draw(border);

    // grid lines
    sf::Vertex line[2];

    // verticals
    for (int x = 1; x < COLS; ++x) {
        const float px = m_origin.x + static_cast<float>(x) * cell;
        line[0] = sf::Vertex(sf::Vector2f{px, m_origin.y}, Colors::Grid);
        line[1] = sf::Vertex(sf::Vector2f{
            px,
            m_origin.y + static_cast<float>(VISIBLE_ROWS) * cell
        }, Colors::Grid);
        m_window.draw(line, 2, sf::PrimitiveType::Lines);
    }

    // horizontals
    for (int y = 1; y < VISIBLE_ROWS; ++y) {
        const float py = m_origin.y + static_cast<float>(y) * cell;
        line[0] = sf::Vertex(sf::Vector2f{m_origin.x, py}, Colors::Grid);
        line[1] = sf::Vertex(sf::Vector2f{
            m_origin.x + static_cast<float>(COLS) * cell,
            py
        }, Colors::Grid);
        m_window.draw(line, 2, sf::PrimitiveType::Lines);
    }
}

void PlayfieldRenderer::drawCells(const GameState& gs)
{
    const float cell = static_cast<float>(CELL);
    sf::RectangleShape rect(sf::Vector2f{cell - 2.f, cell - 2.f});

    for (int y = 0; y < VISIBLE_ROWS; ++y) {
        for (int x = 0; x < COLS; ++x) {
            const auto v = gs.grid[y * COLS + x];
            if (v == 0) continue;

            const int screenRow = VISIBLE_ROWS - 1 - y;
            rect.setFillColor(colorFromCell(v));
            rect.setPosition(sf::Vector2f{
                m_origin.x + static_cast<float>(x) * cell + 1.f,
                m_origin.y + static_cast<float>(screenRow) * cell + 1.f
            });
            m_window.draw(rect);
        }
    }
}

void PlayfieldRenderer::drawActive(const GameState& gs)
{
    const float cell = static_cast<float>(CELL);

    const auto& p  = gs.active;
    const auto& sh = shape(p.type).cells[p.rot];

    sf::RectangleShape rect(sf::Vector2f{cell - 2.f, cell - 2.f});
    rect.setFillColor(color(p.type));

    for (const auto& c : sh) {
        const int gx = p.x + c[0];
        const int gy = p.y + c[1];

        if (gx < 0 || gx >= COLS)          continue;
        if (gy < 0 || gy >= VISIBLE_ROWS)  continue;

        const int screenRow = VISIBLE_ROWS - 1 - gy;
        rect.setPosition(sf::Vector2f{
            m_origin.x + static_cast<float>(gx) * cell + 1.f,
            m_origin.y + static_cast<float>(screenRow) * cell + 1.f
        });
        m_window.draw(rect);
    }
}

void PlayfieldRenderer::drawGhost(const GameState& gs)
{
    const float cell = static_cast<float>(CELL);

    ActivePiece ghost = dropToGround(gs);
    const auto& sh = shape(ghost.type).cells[ghost.rot];

    sf::RectangleShape rect(sf::Vector2f{cell - 2.f, cell - 2.f});
    sf::Color c = Colors::pieceColor(ghost.type);
    c.a = 60;
    rect.setFillColor(c);

    for (const auto& off : sh) {
        const int gx = ghost.x + off[0];
        const int gy = ghost.y + off[1];

        if (gx < 0 || gx >= COLS)          continue;
        if (gy < 0 || gy >= VISIBLE_ROWS)  continue;

        const int visRow = gy;
        const float px = m_origin.x + static_cast<float>(gx) * cell + 1.f;
        const float py = m_origin.y
                       + static_cast<float>(VISIBLE_ROWS - 1 - visRow) * cell
                       + 1.f;

        rect.setPosition(sf::Vector2f{px, py});
        m_window.draw(rect);
    }
}

} // namespace Tetris
