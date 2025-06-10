#include "../class_headers/World.h"
#include "../class_headers/EntityFactory.h"
#include "../class_headers/ConcreteEntityFactory.h"
#include "../class_headers/Player.h"
#include "../class_headers/BerserkOrc.h"
#include "../class_headers/MageOrc.h"
#include "../class_headers/Projectile.h"
#include "../class_headers/MagicProjectile.h"
#include "../class_headers/Platform.h"
#include "../class_headers/GameExceptions.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>

World::World(sf::RenderWindow* win, std::unique_ptr<EntityFactory> factory, SoundManager* sndMgr) :
    window(win),
    entityFactory(std::move(factory)),
    soundManagerPtr(sndMgr),
    playerPtr(nullptr), mageOrcPtr(nullptr) {
    if (!window) {
        throw ConfigurationError("World requires a valid RenderWindow pointer!");
    }
    if (!this->entityFactory) { // Check the member variable
        throw ConfigurationError("World requires a valid EntityFactory instance!");
    }
    try {
        loadResources();
        createInitialEntitiesAndPlayer();
        std::cout << "World initialized successfully." << std::endl;
    } catch (const GameError& e) {
        std::cerr << "FATAL ERROR during World construction: " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "FATAL std::exception during World construction: " << e.what() << std::endl;
        throw GameError("A standard error occurred during world initialization: " + std::string(e.what()));
    } catch (...) {
        std::cerr << "FATAL unknown exception during World construction." << std::endl;
        throw GameError("An unknown error occurred during world initialization.");
    }
}

void World::loadResources() {
    if (!backgroundTexture.loadFromFile("assets/backgrounds/background_1.png")) {
        throw ResourceLoadError("Texture", "assets/backgrounds/background_1.png", "SFML loadFromFile failed for World background.");
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
}

void World::createInitialEntitiesAndPlayer() {
    platforms.emplace_back(600.f, 700.f, 300.f, 20.f);
    platforms.emplace_back(700.f, 500.f, 400.f, 20.f);
    platforms.emplace_back(300.f, 350.f, 250.f, 20.f);

    constexpr float playerFrameH{128.f};
    constexpr float playerScaleY{2.0f};
    constexpr float playerSpriteOriginYRelToFrameTop{playerFrameH / 2.f};
    constexpr float playerCustomHitboxTopLocal = (playerFrameH * (2.f/5.f));
    constexpr float playerCustomHitboxHeightLocal = playerFrameH * (3.f/5.f);
    constexpr float groundY = 900.f;
    float playerLocalHitboxBottomRelToFrameTop = playerCustomHitboxTopLocal + playerCustomHitboxHeightLocal;
    float playerTargetY = groundY - (playerLocalHitboxBottomRelToFrameTop - playerSpriteOriginYRelToFrameTop) * playerScaleY;
    sf::Vector2f playerStartPosition = { 200.f, playerTargetY };


    try {
        playerPtr = &Player::getInstance(window, "idle", "assets/player/Idle.png", 6, 0.1f, playerStartPosition);
        playerPtr->addObserver(soundManagerPtr);
    } catch (const std::exception& e) {
        std::cerr << "ERROR creating Player singleton: " << e.what() << std::endl;
        throw;
    }

    // Orc Y-Positioning
    constexpr float orcFrameH = 96.f;
    constexpr float orcScaleY = 2.0f;
    constexpr float orcHitboxHeightLocal = orcFrameH * 0.6f;
    constexpr float orcHitboxOffsetYLocal = orcFrameH * (1.0f - 0.6f);
    constexpr float orcLocalHitboxBottomRelToFrameTop = orcHitboxOffsetYLocal + orcHitboxHeightLocal;
    constexpr float orcSpriteOriginYRelToFrameTop = orcFrameH / 2.0f;
    float orcTargetY = groundY - (orcLocalHitboxBottomRelToFrameTop - orcSpriteOriginYRelToFrameTop) * orcScaleY;


    // use the factory INTERFACE methods for polymorphic creation
    entities.push_back(entityFactory->makeBerserkOrc(window, sf::Vector2f(500.f, orcTargetY)));
    entities.push_back(entityFactory->makeBerserkOrc(window, sf::Vector2f(1200.f, orcTargetY)));

    sf::Vector2u windowSize = window->getSize();
    sf::Vector2f mageStartPosition = { static_cast<float>(windowSize.x) - 150.f, static_cast<float>(windowSize.y) / 2.5f };

    std::unique_ptr<Entity> mageEntity = entityFactory->makeMageOrc(window, mageStartPosition);
    // dynamic cast to get specific MageOrc pointer
    mageOrcPtr = dynamic_cast<MageOrc*>(mageEntity.get());
    if (!mageOrcPtr && mageEntity) {
        std::cerr << "Warning: Failed to dynamic_cast created MageOrc entity from factory." << std::endl;
    }
    entities.push_back(std::move(mageEntity));
}

void World::handleInput() {
    if (playerPtr && playerPtr->getHealthPoints() > 0) {
        playerPtr->actions();
    } // handle user input (actions of Player)
}

void World::update(float dt) { // global update
    sf::Vector2f currentPlayerPos = {0,0};
    if (playerPtr) {
        if (playerPtr->getHealthPoints() > 0) {
            playerPtr->updater(platforms);
        }
        playerPtr->update();
        currentPlayerPos = playerPtr->getPosition();
    }

    for (auto& entityPtr : entities) {
        if (!entityPtr) continue;
        if (auto* mage = dynamic_cast<MageOrc*>(entityPtr.get())) {
            if (playerPtr) {
                mage->updatePlayerPosition(&currentPlayerPos);
            } else {
                mage->updatePlayerPosition(nullptr);
            }
            mage->updater(dt);
        } else if (auto* orc = dynamic_cast<BerserkOrc*>(entityPtr.get())) {
            orc->update();
        } else { // for other entities like projectiles
            entityPtr->update();
        }
    }

    // spawn all projectiles
    spawnPlayerProjectiles();
    spawnEnemyProjectiles();

    if (playerPtr && playerPtr->getHealthPoints() > 0) {
        checkCollisions();
    }
    removeMarkedEntities();
}

void World::spawnPlayerProjectiles() {
    if (playerPtr && playerPtr->wantsToShootProjectile()) {
        ProjectileSpawnInfo spawnInfo = playerPtr->getProjectileSpawnDetails();
        try { // static template method from ConcreteEntityFactory
            entities.push_back(ConcreteEntityFactory::create<Projectile>(window, spawnInfo.position.x, spawnInfo.position.y, spawnInfo.direction.x, spawnInfo.direction.y, spawnInfo.speed));
        } catch (const GameError& e) {
            std::cerr << "Error spawning Player Projectile: " << e.what() << std::endl;
        }
    }
}

void World::spawnEnemyProjectiles() {
    std::vector<std::unique_ptr<Entity>> newProjectiles;
    if (mageOrcPtr && !mageOrcPtr->isMarkedForRemoval()) {
        std::vector<MagicProjectileSpawnInfo> spawnQueue = mageOrcPtr->getProjectilesToSpawn();
        for (const auto& spawnInfo : spawnQueue) {
            try { // static template method from ConcreteEntityFactory
                newProjectiles.push_back(ConcreteEntityFactory::create<MagicProjectile>(window, spawnInfo.position.x, spawnInfo.position.y, spawnInfo.direction.x, spawnInfo.direction.y, spawnInfo.speed));
            } catch (const GameError& e) {
                std::cerr << "Error spawning MagicProjectile: " << e.what() << std::endl;
            }
        }
    }
    if (!newProjectiles.empty()) {
        std::move(newProjectiles.begin(), newProjectiles.end(), std::back_inserter(entities));
    }
}

void World::checkCollisions() {
    if (playerPtr->getHealthPoints() <= 0) return;

    sf::FloatRect playerHitbox = playerPtr->getCollisionBounds();

    for (const auto& entityPtr : entities) {
        if (!entityPtr) continue;

        // Use auto for dynamic_cast results
        if (auto* orc = dynamic_cast<BerserkOrc*>(entityPtr.get())) { // Requires BerserkOrc.h
            if (!orc->isMarkedForRemoval() && playerHitbox.intersects(orc->getCollisionBounds())) {
                playerPtr->takeDamage();
            }
        } else if (auto* mage = dynamic_cast<MageOrc*>(entityPtr.get())) { // Requires MageOrc.h
            if (!mage->isMarkedForRemoval() && playerHitbox.intersects(mage->getCollisionBounds())) {
                playerPtr->takeDamage();
            }
        } else if (auto* magicBullet = dynamic_cast<MagicProjectile*>(entityPtr.get())) { // Requires MagicProjectile.h
            if (!magicBullet->isMarkedForRemoval() && playerHitbox.intersects(magicBullet->getVisualBounds())) {
                playerPtr->takeDamage();
                magicBullet->markForRemoval();
            }
        }
    }

    for (auto& projectileEntityPtr : entities) {
        if (!projectileEntityPtr) continue;

        if (auto* playerBullet = dynamic_cast<Projectile*>(projectileEntityPtr.get())) { // Requires Projectile.h
            if (playerBullet->isMarkedForRemoval()) continue;

            sf::FloatRect projBounds = playerBullet->getVisualBounds();

            for (auto& targetEntityPtr : entities) {
                if (!targetEntityPtr || targetEntityPtr.get() == playerBullet) continue;

                if (auto* orc = dynamic_cast<BerserkOrc*>(targetEntityPtr.get())) { // Requires BerserkOrc.h
                    if (!orc->isMarkedForRemoval() && projBounds.intersects(orc->getCollisionBounds())) {
                        playerBullet->markForRemoval();
                        orc->takeDamage();
                        break;
                    }
                } else if (auto* mage = dynamic_cast<MageOrc*>(targetEntityPtr.get())) { // Requires MageOrc.h
                    if (!mage->isMarkedForRemoval() && projBounds.intersects(mage->getCollisionBounds())) {
                        playerBullet->markForRemoval();
                        mage->takeDamage();
                        break;
                    }
                }
            }
        }
    }
}

void World::removeMarkedEntities() {
    entities.erase(
        std::remove_if(entities.begin(), entities.end(),
            [](const std::unique_ptr<Entity>& entity) {
                if (!entity) return true;
                // Using auto for dynamic_cast results here too
                if (auto* p = dynamic_cast<Projectile*>(entity.get())) return p->isMarkedForRemoval();
                if (auto* mp = dynamic_cast<MagicProjectile*>(entity.get())) return mp->isMarkedForRemoval();
                if (auto* bo = dynamic_cast<BerserkOrc*>(entity.get())) return bo->isMarkedForRemoval();
                if (auto* mo = dynamic_cast<MageOrc*>(entity.get())) return mo->isMarkedForRemoval();
                return false;
            }),
        entities.end()
    );

    if (mageOrcPtr) {
        bool found = false;
        for (const auto& entity : entities) {
            if (entity.get() == mageOrcPtr) {
                found = true;
                break;
            }
        }
        if (!found) {
            mageOrcPtr = nullptr;
        }
    }
}

void World::draw() {
    window->draw(backgroundSprite);
    for (const auto& platform : platforms) {
        platform.draw(*window);
    }
    for (const auto& entity : entities) {
        if (entity) entity->draw();
    }
    if (playerPtr) {
        playerPtr->draw();
    }
}

bool World::isGameOver() const {
    if (playerPtr) {
        return playerPtr->getHealthPoints() <= 0;
    }
    return true;
}