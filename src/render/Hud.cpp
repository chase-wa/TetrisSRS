#include "render/Hud.hpp"
#include <filesystem>

namespace fs = std::filesystem;
namespace Tetris {

    static fs::path findFont() {
        // Try: working dir, exe dir/resources, project-root/resources
        try {
            // 1) working dir
            if (fs::exists("resources/fonts/DejaVuSans.ttf"))
                return "resources/fonts/DejaVuSans.ttf";

            // 2) exe-dir/resources
            const auto exe = fs::path(std::filesystem::current_path()); // working dir
            // If you want the actual exe path instead, use std::filesystem::read_symlink on /proc/self/exe on Linux
            // and GetModuleFileName on Windows. For now, keep it simple.

            // 3) project root (one level up when running from build/Debug)
            if (fs::exists("../resources/fonts/DejaVuSans.ttf"))
                return "../resources/fonts/DejaVuSans.ttf";
            if (fs::exists("../../resources/fonts/DejaVuSans.ttf"))
                return "../../resources/fonts/DejaVuSans.ttf";
        } catch (...) {}
        return {};
    }

    Hud::Hud(sf::RenderWindow& window)
    : m_window(window), m_fps(m_font) {
        const auto fontPath = findFont();
        if (!fontPath.empty())
            m_fontOk = m_font.openFromFile(fontPath.string());
        else
            m_fontOk = false;

        m_fps.setCharacterSize(16);
        m_fps.setFillColor(sf::Color(200,200,200));
        m_fps.setPosition(sf::Vector2f{8.f, 8.f});
    }

    void Hud::update(float dt) {
        m_accum += dt; ++m_frames;
        if (m_accum >= 0.25f) {
            const int fps = static_cast<int>(m_frames / m_accum);
            m_frames = 0; m_accum = 0.f;
            if (m_fontOk) m_fps.setString(std::to_string(fps) + " FPS");
        }
    }

    void Hud::draw() {
        if (m_fontOk) m_window.draw(m_fps);
    }

} // namespace Tetris
