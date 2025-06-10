#include "../class_headers/Platform.h"
#include <SFML/Graphics.hpp>

// platform constructor only positioning and size needed
Platform::Platform(float x, float y, float width, float height) {
    shape.setSize({width, height});
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(x, y);
}

// platform draw function
void Platform::draw(sf::RenderWindow& win) const { win.draw(shape); }

// getter for collisions
sf::FloatRect Platform::getBounds() const { return shape.getGlobalBounds(); }