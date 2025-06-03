#include "../class_headers/Projectile.h"
#include "../class_headers/Entity.h"
#include <SFML/Graphics.hpp>
#include <cmath>

Projectile::Projectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float projectileSpeed) :
    Entity(win), speed(projectileSpeed) {
    this->frameWidth = 10;
    this->frameHeight = 3;
    this->healthPoints = 1;

    loadAnimationTexture("projectile", "assets/projectiles/player_bullet.png");
    setAnimation("projectile", 1, 1.0f);

    float magnitude = std::sqrt(dx * dx + dy * dy);
    if (magnitude > 0) {
        velocity = sf::Vector2f(dx / magnitude * speed, dy / magnitude * speed);
    } else {
        velocity = sf::Vector2f(speed, 0.0f);
    }

    setScale(2.75f, 2.75f);
    setPosition(x, y);
}


void Projectile::actions() {}
void Projectile::takeDamage() {}

void Projectile::update() {
    sprite.move(velocity);
    checkOffScreen();
    Entity::update();
}

void Projectile::draw() {
    if (window && sprite.getTexture() != nullptr) {
        window->draw(this->sprite);
    }
}

void Projectile::markForRemoval() { markedForRemoval = true; }

bool Projectile::isMarkedForRemoval() const { return markedForRemoval; }

void Projectile::checkOffScreen() {
    if (window && !markedForRemoval) {
        sf::Vector2u windowSize = window->getSize();
        sf::FloatRect bounds = getVisualBounds();
        float buffer = std::max(bounds.width, bounds.height) * 2.0f;

        if (bounds.left + bounds.width < -buffer || bounds.left > static_cast<float>(windowSize.x) + buffer ||
            bounds.top + bounds.height < -buffer || bounds.top > static_cast<float>(windowSize.y) + buffer)
        { markForRemoval(); }
    }
}