#ifndef WORLD_H
#define WORLD_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>

#include "Entity.h"
#include "Player.h"
#include "BerserkOrc.h"
#include "Projectile.h"
#include "MageOrc.h"
#include "MagicProjectile.h"
#include "Platform.h"
#include "GameExceptions.h"

class World {
    sf::RenderWindow* window;
    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<Platform> platforms;
    Player* playerPtr = nullptr;
    MageOrc* mageOrcPtr = nullptr;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    void loadResources() {
        try {
            if (!backgroundTexture.loadFromFile("assets/backgrounds/background_1.png")) {
                throw ResourceLoadError("Texture", "assets/backgrounds/background_1.png", "SFML loadFromFile failed in World.");
            }
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
        } catch (const ResourceLoadError& e) {
            std::cerr << "World Resource Error: " << e.what() << std::endl;
            throw;
        }
    }

    void createInitialEntities() {
        try {
            platforms.emplace_back(600.f, 700.f, 300.f, 20.f);
            platforms.emplace_back(700.f, 500.f, 400.f, 20.f);
            platforms.emplace_back(300.f, 350.f, 250.f, 20.f);

            float playerFrameH = 128.f;
            float playerScaleY = 2.0f;
            float hitboxH_local = playerFrameH * (3.0f / 5.0f);
            float hitboxOffsetY_local = playerFrameH - hitboxH_local;
            float groundY = 900.f;
            float hitboxBottomTarget = groundY - 1.f;
            float playerCenterY = hitboxBottomTarget - (hitboxOffsetY_local + hitboxH_local - playerFrameH / 2.0f) * playerScaleY;
            sf::Vector2f playerStartPosition = { 200.f, playerCenterY };

            auto player = std::make_unique<Player>(window, "idle", "assets/player/Idle.png", 6, 0.1f, playerStartPosition);
            playerPtr = player.get();
            entities.push_back(std::move(player));

            float orcFrameH = 96.f;
            float orcScaleY = 2.0f;
            float orcOriginOffsetY = orcFrameH / 2.0f;
            float orcFeetOffset = orcOriginOffsetY * orcScaleY;
            entities.push_back(std::make_unique<BerserkOrc>(window, sf::Vector2f(500.f, groundY - orcFeetOffset), 200.f, 2.f));
            entities.push_back(std::make_unique<BerserkOrc>(window, sf::Vector2f(1200.f, groundY - orcFeetOffset), 200.f, 1.5f));

            sf::Vector2u windowSize = window->getSize();
            sf::Vector2f mageStartPosition = {
                static_cast<float>(windowSize.x) - 150.f,
                static_cast<float>(windowSize.y) / 2.f
            };
            auto mage = std::make_unique<MageOrc>(window, mageStartPosition);
            mageOrcPtr = mage.get();
            entities.push_back(std::move(mage));
        } catch (const GameError& e) {
            std::cerr << "ERROR during World entity creation: " << e.what() << std::endl;
            throw;
        }
    }

    void checkCollisions() {
        if (!playerPtr) { throw GameLogicError("Player pointer is null during collision check."); }

        sf::FloatRect playerHitbox = playerPtr->getCollisionBounds();
        // std::vector<Entity*> projectilesToRemove;

        for (const auto& entity : entities) {
            if (!entity || entity.get() == playerPtr) continue;
            if (const BerserkOrc* orc = dynamic_cast<const BerserkOrc*>(entity.get())) {
                 if (!orc->isMarkedForRemoval() && playerHitbox.intersects(orc->getCollisionBounds())) {
                    std::cout << "Collision: Player <-> BerserkOrc!" << std::endl;
                    playerPtr->takeDamage();
                 }
            }
            else if (const MageOrc* mage = dynamic_cast<const MageOrc*>(entity.get())) {
                 if (!mage->isMarkedForRemoval() && playerHitbox.intersects(mage->getCollisionBounds())) {
                    std::cout << "Collision: Player <-> MageOrc!" << std::endl;
                    playerPtr->takeDamage();
                 }
            }
            else if (MagicProjectile* magicBullet = dynamic_cast<MagicProjectile*>(entity.get())) {
                if (!magicBullet->isMarkedForRemoval() && playerHitbox.intersects(magicBullet->getVisualBounds())) { // Use visual bounds for projectiles? Or define hitbox?
                    std::cout << "Collision: Player <-> MagicProjectile!" << std::endl;
                    playerPtr->takeDamage();
                    magicBullet->markForRemoval();
                }
            }
        }

        for (auto& entity : entities) {
            if (Projectile* playerBullet = dynamic_cast<Projectile*>(entity.get())) {
                if (!playerBullet->isMarkedForRemoval()) {
                    sf::FloatRect projBounds = playerBullet->getVisualBounds();

                    for (auto& targetEntity : entities) {
                        if (!targetEntity || targetEntity.get() == playerBullet) continue;

                        if (BerserkOrc* orc = dynamic_cast<BerserkOrc*>(targetEntity.get())) {
                            if (!orc->isMarkedForRemoval() && projBounds.intersects(orc->getCollisionBounds())) {
                                std::cout << "Collision: Player Projectile -> BerserkOrc!" << std::endl;
                                playerBullet->markForRemoval();
                                orc->takeDamage();
                                break;
                            }
                        } else if (MageOrc* mage = dynamic_cast<MageOrc*>(targetEntity.get())) {
                             if (!mage->isMarkedForRemoval() && projBounds.intersects(mage->getCollisionBounds())) {
                                std::cout << "Collision: Player Projectile -> MageOrc!" << std::endl;
                                playerBullet->markForRemoval();
                                mage->takeDamage();
                                break;
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
                if (!entity) return true;

                if (const Projectile* p = dynamic_cast<const Projectile*>(entity.get())) {
                    return p->isMarkedForRemoval();
                }
                if (const MagicProjectile* mp = dynamic_cast<const MagicProjectile*>(entity.get())) {
                    return mp->isMarkedForRemoval();
                }
                if (const BerserkOrc* bo = dynamic_cast<const BerserkOrc*>(entity.get())) {
                    return bo->isMarkedForRemoval();
                }
                if (const MageOrc* mo = dynamic_cast<const MageOrc*>(entity.get())) {
                    return mo->isMarkedForRemoval();
                }
                // if (const Player* pl = dynamic_cast<const Player*>(entity.get())) {
                //     return pl->isMarkedForRemoval(); // Assuming Player has this method
                // }
                return false;
            }),
            entities.end()
        );

        // if (playerPtr && playerPtr->isMarkedForRemoval()) playerPtr = nullptr;
        if (mageOrcPtr && mageOrcPtr->isMarkedForRemoval()) mageOrcPtr = nullptr;
    }

    void spawnRequestedProjectiles() {
        std::vector<std::unique_ptr<Entity>> newProjectiles;

        if (mageOrcPtr && !mageOrcPtr->isMarkedForRemoval()) {
            std::vector<MagicProjectileSpawnInfo> spawnQueue = mageOrcPtr->getProjectilesToSpawn();
            for (const auto& spawnInfo : spawnQueue) {
                try {
                    newProjectiles.push_back(std::make_unique<MagicProjectile>(
                        window,
                        spawnInfo.position.x, spawnInfo.position.y,
                        spawnInfo.direction.x, spawnInfo.direction.y,
                        spawnInfo.speed
                    ));
                } catch (const GameError& e) {
                    std::cerr << "Error spawning MagicProjectile: " << e.what() << std::endl;
                }
            }
        }

        if (!newProjectiles.empty()) { std::move(newProjectiles.begin(), newProjectiles.end(), std::back_inserter(entities)); }
    }

public:
    explicit World(sf::RenderWindow* win) : window(win) {
        if (!window) {
            throw ConfigurationError("World requires a valid RenderWindow pointer!");
        }
        loadResources();
        createInitialEntities();
        std::cout << "World initialized successfully." << std::endl;
    }

    void handleInput() {
        if (playerPtr) {}
    }

    void update(float dt) {
        sf::Vector2f currentPlayerPos;
        if (playerPtr) {
            playerPtr->actions();
            currentPlayerPos = playerPtr->getPosition();

            if (playerPtr->wantsToShootProjectile()) {
                ProjectileSpawnInfo spawnInfo = playerPtr->getProjectileSpawnDetails();
                try {
                    entities.push_back(std::make_unique<Projectile>(window, spawnInfo.position.x, spawnInfo.position.y, spawnInfo.direction.x, spawnInfo.direction.y, spawnInfo.speed));
                } catch (const GameError& e) {
                    std::cerr << "Error spawning Player Projectile: " << e.what() << std::endl;
                }
            }
        }

        for (auto& entity : entities) {
             if (!entity) continue;

             if (Player* p = dynamic_cast<Player*>(entity.get())) {
                 p->updater(platforms);
             } else if (MageOrc* mage = dynamic_cast<MageOrc*>(entity.get())) {
                  mage->updatePlayerPosition(&currentPlayerPos);
                  mage->updater(dt);
             } else if (BerserkOrc* orc = dynamic_cast<BerserkOrc*>(entity.get())) {
                  orc->update();
             }
              else { entity->update(); }
        }

        spawnRequestedProjectiles();
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