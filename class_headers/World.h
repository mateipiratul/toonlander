#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <algorithm>

#include "Entity.h"
#include "Player.h"
#include "BerserkOrc.h"
#include "Projectile.h"
#include "Platform.h"

class World {
    sf::RenderWindow* window;
    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<Platform> platforms;
    Player* playerPtr = nullptr;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    void loadResources() {
        if (!backgroundTexture.loadFromFile("assets/backgrounds/background_1.png")) {
            std::cerr << "Failed to load background image in World\n";
        } else {
            backgroundSprite.setTexture(backgroundTexture);
            sf::Vector2u textureSize = backgroundTexture.getSize();
            sf::Vector2u windowSize = window->getSize();
            float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
            float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
            float bgScale = std::max(scaleX, scaleY);
            backgroundSprite.setScale(bgScale, bgScale);
            backgroundSprite.setPosition(
                (static_cast<float>(windowSize.x) - backgroundSprite.getGlobalBounds().width) / 2.f,
                (static_cast<float>(windowSize.y) - backgroundSprite.getGlobalBounds().height) / 2.f
            );
        }
    }

    void createInitialEntities() {
        platforms.emplace_back(600.f, 700.f, 300.f, 20.f);
        platforms.emplace_back(700.f, 500.f, 400.f, 20.f);
        platforms.emplace_back(300.f, 350.f, 250.f, 20.f);

        float playerFrameH = 128.f;
        float playerScaleY = 2.0f;
        float hitboxH_local = playerFrameH * (3.0f/5.0f);
        float hitboxOffsetY_local = playerFrameH - hitboxH_local;
        float hitboxBottomTarget = 900.f - 1.f;
        float playerCenterY = hitboxBottomTarget - (hitboxOffsetY_local + hitboxH_local / 2.0f) * playerScaleY;
        sf::Vector2f playerStartPosition = { 200.f, playerCenterY };

        auto player = std::make_unique<Player>(window, "idle", "assets/player/Idle.png", 6, 0.1f, playerStartPosition);
        playerPtr = player.get();
        entities.push_back(std::move(player));

        float orcFrameH = 96.f;
        float orcScaleY = 2.0f;
        float orcOriginOffsetY = orcFrameH / 2.0f;
        float orcFeetOffset = orcOriginOffsetY * orcScaleY;

        entities.push_back(std::make_unique<BerserkOrc>(window, sf::Vector2f(500.f, 900.f - orcFeetOffset), 200.f, 2.f));
        entities.push_back(std::make_unique<BerserkOrc>(window, sf::Vector2f(1200.f, 900.f - orcFeetOffset), 200.f, 1.5f));
    }

    void checkCollisions() {
        if (!playerPtr) return;
        sf::FloatRect playerHitbox = playerPtr->getCollisionBounds();

        for (const auto& entity : entities) {
            if (BerserkOrc* orc = dynamic_cast<BerserkOrc*>(entity.get())) {
                 if (!orc->isMarkedForRemoval()) {
                     if (playerHitbox.intersects(orc->getCollisionBounds())) {
                        std::cout << "Collision: Player <-> Orc!" << std::endl;
                        playerPtr->takeDamage();
                     }
                 }
            }
        }

        for (auto& proj_entity : entities) {
             if (Projectile* projectile = dynamic_cast<Projectile*>(proj_entity.get())) {
                 if (!projectile->isMarkedForRemoval()) {
                     sf::FloatRect projBounds = projectile->getVisualBounds();

                     for (auto& target_entity : entities) {
                         if (BerserkOrc* orc = dynamic_cast<BerserkOrc*>(target_entity.get())) {
                             if (!orc->isMarkedForRemoval()) {
                                 if (projBounds.intersects(orc->getCollisionBounds())) {
                                     std::cout << "Collision: Projectile -> Orc!" << std::endl;
                                     projectile->markForRemoval();
                                     orc->takeDamage();
                                     break;
                                 }
                             }
                         }
                     }
                 }
            }
        }
    }

    void removeMarkedEntities() {
         entities.erase(
             std::remove_if(entities.begin(), entities.end(), [](const std::unique_ptr<Entity>& entity) {
                 if (const Projectile* p = dynamic_cast<const Projectile*>(entity.get())) {
                     return p->isMarkedForRemoval();
                 }
                 if (const BerserkOrc* o = dynamic_cast<const BerserkOrc*>(entity.get())) {
                     return o->isMarkedForRemoval();
                 }
                 return false;
             }),
             entities.end()
         );
    }


public:
    explicit World(sf::RenderWindow* win) : window(win) {
        if (!window) {
            throw std::runtime_error("World requires a valid RenderWindow pointer!");
        }
        loadResources();
        createInitialEntities();
    }

    void handleInput(const sf::Event& event) {
        if (playerPtr) {}
    }

    void update(float dt) {
        if (playerPtr) {
            playerPtr->actions();

            if (playerPtr->wantsToShootProjectile()) {
                ProjectileSpawnInfo spawnInfo = playerPtr->getProjectileSpawnDetails();
                entities.push_back(std::make_unique<Projectile>(window, spawnInfo.position.x, spawnInfo.position.y, spawnInfo.direction.x, spawnInfo.direction.y, spawnInfo.speed));
            }
        }

        for (auto& entity : entities) {
             if (Player* p = dynamic_cast<Player*>(entity.get())) {
                 p->update(dt, platforms);
             } else if (BerserkOrc* o = dynamic_cast<BerserkOrc*>(entity.get())) {
                  o->update(dt);
             } else {
                 entity->update(dt);
             }
        }

        checkCollisions();
        removeMarkedEntities();
    }

    void draw() {
        window->draw(backgroundSprite);
        for (const auto& platform : platforms) { platform.draw(*window); }
        for (const auto& entity : entities) { entity->draw(); }
    }
};

#endif // WORLD_H