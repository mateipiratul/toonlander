#ifndef PLATFORM_H
#define PLATFORM_H
#include <SFML/Graphics.hpp>

class Platform {
    sf::RectangleShape shape;

public:
    Platform(float x, float y, float width, float height) {
        shape.setSize({width, height});
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x, y);
    }

    void draw(sf::RenderWindow& win) const { win.draw(shape); }

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

#endif //PLATFORM_H