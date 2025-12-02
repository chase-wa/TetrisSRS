// tests/smoketest.cpp
#include <SFML/Graphics/RenderTexture.hpp>

int main() {
    sf::RenderTexture rt({64u, 64u}); // SFML 3: construct with size
    rt.clear();
    rt.display();
    return 0;
}
