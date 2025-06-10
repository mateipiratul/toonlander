#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "SoundManager.h"
#include "Platform.h"

// forward declarations
class Entity;
class Player;
class MageOrc;
class EntityFactory;



class World {
    sf::RenderWindow* window;
    std::unique_ptr<EntityFactory> entityFactory; // hold the factory
    std::vector<std::unique_ptr<Entity>> entities; // entities created by factory
    std::vector<Platform> platforms; // separate vector for static platforms
    SoundManager* soundManagerPtr; // observer for sounds
    Player* playerPtr{nullptr}; // pointer to Player entity (singleton)
    MageOrc* mageOrcPtr{nullptr}; // pointer to MageOrc entity (simulate a singleton)

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    void loadResources(); // load textures and assets
    void createInitialEntitiesAndPlayer(); // player and enemies setup
    void checkCollisions(); // handle all collisions
    void removeMarkedEntities(); // delete dead or old entities
    void spawnPlayerProjectiles(); // Projectile entity handler
    void spawnEnemyProjectiles(); // MagicProjectile entity handler

public:
    explicit World(sf::RenderWindow* win, std::unique_ptr<EntityFactory> factory, SoundManager* soundManagerPtr);
    void handleInput(); // handle player input
    void update(float dt); // call all update functions
    void draw(); // call all draw functions

    bool isGameOver() const; // getter for game over
};

#endif // WORLD_H