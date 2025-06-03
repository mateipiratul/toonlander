#ifndef CONCRETEENTITYFACTORY_H
#define CONCRETEENTITYFACTORY_H

#include "EntityFactory.h"
// include concrete entity headers this factory will create
#include "BerserkOrc.h"
#include "MageOrc.h"
#include "Projectile.h"
#include "MagicProjectile.h"
#include <utility>

class ConcreteEntityFactory : public EntityFactory {
public:
    ~ConcreteEntityFactory() override = default;

    // implementation of specific factory methods from the interface
    std::unique_ptr<Entity> makeBerserkOrc(sf::RenderWindow* win, const sf::Vector2f& pos) override;
    std::unique_ptr<Entity> makeMageOrc(sf::RenderWindow* win, const sf::Vector2f& pos) override;
    std::unique_ptr<Entity> makeProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) override;
    std::unique_ptr<Entity> makeMagicProjectile(sf::RenderWindow* win, float x, float y, float dx, float dy, float speed) override;

    template <typename T, typename... Args>
    static std::unique_ptr<T> create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
};

#endif // CONCRETEENTITYFACTORY_H