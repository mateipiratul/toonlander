#ifndef MAGICPROJECTILE_H
#define MAGICPROJECTILE_H
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

#include "Entity.h"
#include "GameExceptions.h"

class MagicProjectile : public Entity {
    float speed;
    bool markedForRemoval = false;

public:
    MagicProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float projectileSpeed)
        : Entity(win),
          speed(projectileSpeed)
    {
        this->frameWidth = 20;
        this->frameHeight = 20;
        this->healthPoints = 1;

        try {
            loadAnimationTexture("mage_projectile", "assets/projectiles/mage_projectile.png");
            setAnimation("mage_projectile", 8, 0.15f);

            float magnitude = std::sqrt(dx * dx + dy * dy);
            if (magnitude > 0) {
                velocity = sf::Vector2f((dx / magnitude) * speed, (dy / magnitude) * speed);
            } else {
                velocity = sf::Vector2f(speed, 0.0f);
                std::cerr << "Warning: MagicProjectile created with zero direction vector." << std::endl;
            }

            setScale(1.5f, 1.5f);
            setPosition(x, y);

        } catch (const ResourceLoadError& e) {
            std::cerr << "ERROR creating MagicProjectile (ResourceLoadError): " << e.what() << std::endl;
            throw GameError("Failed to initialize MagicProjectile resources.");
        } catch (const InvalidStateError& e) {
             std::cerr << "ERROR creating MagicProjectile (InvalidStateError): " << e.what() << std::endl;
             throw GameError("Failed to configure MagicProjectile state.");
        }
    }

    void update() override {
        sprite.move(velocity);
        checkOffScreen();
        Entity::update();
    }

    void draw() override { if (window && sprite.getTexture() != nullptr) { window->draw(this->sprite); } }

    void markForRemoval() { markedForRemoval = true; }

    bool isMarkedForRemoval() const { return markedForRemoval; }

    void checkOffScreen() {
         if (window && !markedForRemoval) {
            sf::Vector2u windowSize = window->getSize();
            sf::FloatRect bounds = getVisualBounds();
            float buffer = std::max(bounds.width, bounds.height) * 2.0f;

            if (bounds.left + bounds.width < -buffer || bounds.left > static_cast<float>(windowSize.x) + buffer ||
                bounds.top + bounds.height < -buffer || bounds.top > static_cast<float>(windowSize.y) + buffer)
            { markForRemoval(); }
        }
    }

    void actions() override {}
    void takeDamage() override { markForRemoval(); }
};

#endif //MAGICPROJECTILE_H