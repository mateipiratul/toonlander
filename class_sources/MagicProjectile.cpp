#include "../class_headers/MagicProjectile.h"
#include "../class_headers/Entity.h"
#include "../class_headers/GameExceptions.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <iostream>

MagicProjectile::MagicProjectile(sf::RenderWindow *win, float x, float y, float dx, float dy, float projectileSpeed) :
    Entity(win),
    speed(projectileSpeed) {
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

void MagicProjectile::update() {
    sprite.move(velocity);
    checkOffScreen(); // remove entity
    Entity::update(); // call base function
}

void MagicProjectile::draw() { if (window && sprite.getTexture() != nullptr) { window->draw(this->sprite); } }

void MagicProjectile::markForRemoval() { markedForRemoval = true; }

bool MagicProjectile::isMarkedForRemoval() const { return markedForRemoval; }

void MagicProjectile::checkOffScreen() {
    if (window && !markedForRemoval) {
        sf::Vector2u windowSize = window->getSize();
        sf::FloatRect bounds = getVisualBounds();
        float buffer = std::max(bounds.width, bounds.height) * 2.0f;

        if (bounds.left + bounds.width < -buffer || bounds.left > static_cast<float>(windowSize.x) + buffer ||
            bounds.top + bounds.height < -buffer || bounds.top > static_cast<float>(windowSize.y) + buffer)
        { markForRemoval(); }
    }
}

void MagicProjectile::actions() {}

void MagicProjectile::takeDamage() { markForRemoval(); }