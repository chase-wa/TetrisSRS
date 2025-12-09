#include "render/Hud.hpp"
#include "game/Pieces.hpp"      // Tetromino, shape()
#include "game/Logic.hpp"       // peekNextPieces, RunType, etc.
#include "render/Colors.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

namespace Tetris {

// ---- font search ----
static fs::path findFont() {
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

Hud::Hud(sf::RenderWindow& window)
: m_window(window)
{
    const auto fontPath = findFont();
    if (!fontPath.empty() && m_font.openFromFile(fontPath.string())) {
        m_fontOk = true;

        // FPS (top-left)
        m_fps = std::make_unique<sf::Text>(m_font, "", 16);
        m_fps->setFillColor(sf::Color(200, 200, 200));
        m_fps->setPosition(sf::Vector2f{8.f, 8.f});

        // Lines
        m_linesText = std::make_unique<sf::Text>(m_font, "", 18);
        m_linesText->setFillColor(sf::Color(230, 230, 230));
        m_linesText->setPosition(sf::Vector2f{8.f, 32.f});

        // "SPRINT 40" label
        m_sprintText = std::make_unique<sf::Text>(m_font, "SPRINT 40", 20);
        m_sprintText->setFillColor(sf::Color(230, 230, 0));
        m_sprintText->setPosition(sf::Vector2f{8.f, 56.f});

        // sprint info (time + finished)
        m_sprintInfo = std::make_unique<sf::Text>(m_font, "", 18);
        m_sprintInfo->setFillColor(sf::Color(230, 230, 230));
        m_sprintInfo->setPosition(sf::Vector2f{8.f, 80.f});

    } else {
        m_fontOk = false;
        m_fps.reset();
        m_linesText.reset();
        m_sprintText.reset();
        m_sprintInfo.reset();
    }
}

void Hud::update(float dt) {
    m_accum += dt;
    ++m_frames;
    if (m_accum >= 0.25f) {
        const int fps = static_cast<int>(m_frames / m_accum);
        m_frames = 0;
        m_accum  = 0.f;
        if (m_fontOk && m_fps)
            m_fps->setString(std::to_string(fps) + " FPS");
    }
}

// Draw a mini piece (rotation 0) centered in a box.
static void drawMiniPieceInBox(sf::RenderWindow& window,
                               Tetromino t,
                               float boxX,
                               float boxY,
                               float boxW,
                               float boxH)
{
    const auto& sh = shape(t).cells[0];

    int minX =  999, maxX = -999;
    int minY =  999, maxY = -999;
    for (const auto& c : sh) {
        int cx = static_cast<int>(c[0]);
        int cy = static_cast<int>(c[1]);
        minX = std::min(minX, cx);
        maxX = std::max(maxX, cx);
        minY = std::min(minY, cy);
        maxY = std::max(maxY, cy);
    }

    const float padding = 8.f;
    const float maxW = boxW - 2.f * padding;
    const float maxH = boxH - 2.f * padding;

    const float pieceCols = static_cast<float>(maxX - minX + 1);
    const float pieceRows = static_cast<float>(maxY - minY + 1);

    float cellSize = 16.f;
    if (pieceCols > 0.f && pieceRows > 0.f) {
        const float fitW = maxW / pieceCols;
        const float fitH = maxH / pieceRows;
        cellSize = std::min(fitW, fitH);
    }

    const float pieceW = pieceCols * cellSize;
    const float pieceH = pieceRows * cellSize;

    const float originX = boxX + (boxW - pieceW) * 0.5f;
    const float originY = boxY + (boxH - pieceH) * 0.5f;

    sf::RectangleShape rect(sf::Vector2f{cellSize - 2.f, cellSize - 2.f});
    rect.setFillColor(Colors::pieceColor(t));

    for (const auto& c : sh) {
        int cx = static_cast<int>(c[0]);
        int cy = static_cast<int>(c[1]);

        float px = originX + (cx - minX) * cellSize;
        float py = originY + (maxY - cy) * cellSize; // flip y for SFML

        rect.setPosition(sf::Vector2f{px + 1.f, py + 1.f});
        window.draw(rect);
    }
}

void Hud::draw(const GameState& state)
{
    // --- update sprint HUD text ---
    if (m_fontOk && m_linesText) {
    	m_linesText->setString("Lines: " + std::to_string(state.totalLinesCleared));
	}

    if (m_fontOk && state.runType == RunType::Sprint && m_sprintInfo) {
        std::ostringstream oss;
        oss.setf(std::ios::fixed);
        oss.precision(2);
        oss << "Time: " << state.sprintTime << " s";
        if (state.gameOver && state.totalLinesCleared >= 40) {
            oss << "\nFinished!";
        }
        m_sprintInfo->setString(oss.str());
    }

    // draw text overlays
    if (m_fontOk) {
        if (m_fps)        m_window.draw(*m_fps);
        if (m_linesText)  m_window.draw(*m_linesText);
        if (state.runType == RunType::Sprint) {
            if (m_sprintText)  m_window.draw(*m_sprintText);
            if (m_sprintInfo)  m_window.draw(*m_sprintInfo);
        }
    }

    // then hold / queue UI
    drawHold(state);
    drawNext(state);
}

void Hud::drawHold(const GameState& state) {
    if (!state.hasHold)
        return;

    const float boxX = 16.f;
    const float boxY = 140.f; // moved down a bit for sprint text
    const float boxW = 96.f;
    const float boxH = 96.f;

    sf::RectangleShape box(sf::Vector2f{boxW, boxH});
    box.setPosition(sf::Vector2f{boxX, boxY});
    box.setFillColor(sf::Color(0, 0, 0, 80));
    box.setOutlineThickness(1.f);
    box.setOutlineColor(sf::Color(80, 80, 80));
    m_window.draw(box);

    drawMiniPieceInBox(m_window, state.holdType, boxX, boxY, boxW, boxH);
}

void Hud::drawNext(const GameState& state) {
    constexpr int maxShown = 5;
    auto upcoming = peekNextPieces<maxShown>(state);

    const float slotH   = 64.f;
    const float boxW    = 96.f;
    const float boxH    = maxShown * slotH;

    const float startX  = static_cast<float>(m_window.getSize().x) - boxW - 16.f;
    const float startY  = 140.f;

    sf::RectangleShape outer(sf::Vector2f{boxW, boxH});
    outer.setPosition(sf::Vector2f{startX, startY});
    outer.setFillColor(sf::Color(0, 0, 0, 80));
    outer.setOutlineThickness(1.f);
    outer.setOutlineColor(sf::Color(80, 80, 80));
    m_window.draw(outer);

    for (int i = 0; i < maxShown; ++i) {
        Tetromino t = upcoming[static_cast<std::size_t>(i)];

        float slotX = startX;
        float slotY = startY + i * slotH;
        drawMiniPieceInBox(m_window, t, slotX, slotY, boxW, slotH);
    }
}

} // namespace Tetris
