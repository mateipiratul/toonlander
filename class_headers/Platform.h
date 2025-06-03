#ifndef PLATFORM_H
#define PLATFORM_H

#include <SFML/Graphics.hpp>

class Platform {
    sf::RectangleShape shape;

public:
    Platform(float x, float y, float width, float height);

    void draw(sf::RenderWindow& win) const;
    sf::FloatRect getBounds() const;
};

#endif //PLATFORM_H