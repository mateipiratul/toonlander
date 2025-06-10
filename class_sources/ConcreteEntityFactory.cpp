#include "../class_headers/ConcreteEntityFactory.h"
// specific factory methods

std::unique_ptr<Entity> ConcreteEntityFactory::makeBerserkOrc(sf::RenderWindow* win, const sf::Vector2f& pos) {
    return std::make_unique<BerserkOrc>(win, pos);
}

std::unique_ptr<Entity> ConcreteEntityFactory::makeMageOrc(sf::RenderWindow* win, const sf::Vector2f& pos) {
    return std::make_unique<MageOrc>(win, pos);
}

std::unique_ptr<Entity> ConcreteEntityFactory::makeProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) {
    return std::make_unique<Projectile>(win, x, y, dx, dy, speed);
}

std::unique_ptr<Entity> ConcreteEntityFactory::makeMagicProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) {
    return std::make_unique<MagicProjectile>(win, x, y, dx, dy, speed);
}