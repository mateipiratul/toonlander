#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <memory>
#include "SFML/Graphics.hpp"

class Entity;


class EntityFactory {
public:
    virtual ~EntityFactory() = default;

    virtual std::unique_ptr<Entity> makeBerserkOrc(sf::RenderWindow* win, const sf::Vector2f& pos) = 0;
    virtual std::unique_ptr<Entity> makeMageOrc(sf::RenderWindow* win, const sf::Vector2f& pos) = 0;
    virtual std::unique_ptr<Entity> makeProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) = 0;
    virtual std::unique_ptr<Entity> makeMagicProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) = 0;
};

#endif // ENTITYFACTORY_H