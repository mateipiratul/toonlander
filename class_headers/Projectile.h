#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

class Projectile : public Entity {
    float speed;
    bool markedForRemoval{false};

public:
    Projectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float projectileSpeed);

    // no implementations for member functions
    void actions() override;
    void takeDamage() override;
    void update() override;
    void draw() override;
    void markForRemoval();
    bool isMarkedForRemoval() const;
    void checkOffScreen();
};

#endif // PROJECTILE_H