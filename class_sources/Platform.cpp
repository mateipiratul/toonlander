#include "../class_headers/Platform.h"
#include <SFML/Graphics.hpp>

Platform::Platform(float x, float y, float width, float height) {
    shape.setSize({width, height});
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(x, y);
}

void Platform::draw(sf::RenderWindow& win) const { win.draw(shape); }

sf::FloatRect Platform::getBounds() const { return shape.getGlobalBounds(); }