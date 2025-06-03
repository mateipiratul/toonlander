#ifndef MAGICPROJECTILE_H
#define MAGICPROJECTILE_H

#include <SFML/Graphics.hpp>
#include "Entity.h"

class MagicProjectile : public Entity {
    float speed;
    bool markedForRemoval{false};

public:
    MagicProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float projectileSpeed);

    // override base class functions
    void update () override;
    void draw() override;
    void actions() override;
    void takeDamage() override;

    void markForRemoval();
    bool isMarkedForRemoval() const;
    void checkOffScreen();
};

#endif //MAGICPROJECTILE_H