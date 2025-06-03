#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
class Entity;
class Player;
#include "Platform.h"
class MageOrc;
class EntityFactory;

class World {
    sf::RenderWindow* window;
    std::unique_ptr<EntityFactory> entityFactory; // hold the factory
    std::vector<std::unique_ptr<Entity>> entities; // entities created by factory
    std::vector<Platform> platforms;
    Player* playerPtr{nullptr};
    MageOrc* mageOrcPtr{nullptr};

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    void loadResources();
    void createInitialEntitiesAndPlayer();
    void checkCollisions();
    void removeMarkedEntities();
    void spawnPlayerProjectiles();
    void spawnEnemyProjectiles();

public:
    explicit World(sf::RenderWindow* win, std::unique_ptr<EntityFactory> factory);

    void handleInput();
    void update(float dt);
    void draw();

    bool isGameOver() const;
};

#endif // WORLD_H