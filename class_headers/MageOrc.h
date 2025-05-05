#ifndef MAGEORC_H
#define MAGEORC_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

#include "Entity.h"
#include "MagicProjectile.h"
#include "GameExceptions.h"

# define M_PI 3.14159265358979323846

struct MagicProjectileSpawnInfo {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed;
};

class MageOrc : public Entity {
    enum class State { IDLE, FLYING, BARRAGE_PREPARE, BARRAGE_FIRE, FLURRY, ARTILLERY};
    State currentState;

    sf::Clock stateTimer;
    float currentStateDuration;
    sf::Clock actionTimer;
    float timeSinceLastAction;
    std::mt19937 rng;

    float flyAmplitudeY;
    float flyFrequencyY;
    float idleAmplitudeY;
    float idleFrequencyY;
    float currentCenterY;
    float targetY;
    float moveSpeedY = 100.f;

    const int barrageCount = 12;
    const float barrageRadius = 100.f;
    const float barrageProjectileSpeed = 3.f;
    const float flurryShotInterval = 0.15f;
    float flurryProjectileSpeed = 9.f;
    float flurryAimVariance = 0.2f;
    sf::Vector2f* playerPositionPtr = nullptr;

    std::vector<MagicProjectileSpawnInfo> projectilesToSpawn;

    sf::RectangleShape hitboxShape;
    sf::FloatRect customHitbox;
    bool showHitbox = true;

    bool markedForRemoval = false;
    bool isAlive = true;

    void chooseNextState() {
        if (!isAlive) {
            currentState = State::IDLE;
            velocity = {0, 0};
            return;
        }

        std::uniform_int_distribution<int> stateChoiceDist(1,10);
        int choice = stateChoiceDist(rng);

        actionTimer.restart();
        timeSinceLastAction = 0.f;

        if (choice == 1) { // idle
            currentState = State::IDLE;
            setAnimation("idle", 4, 0.2f);
            velocity = {0, 0};
            currentCenterY = getPosition().y;
            currentStateDuration = std::uniform_real_distribution<float>(2.f, 3.f)(rng);
            std::cout << "MageOrc: Entering IDLE state for " << currentStateDuration << " s.\n";
        } else if (choice <= 3) { // flying
            currentState = State::FLYING;
            setAnimation("fly", 4, 0.2f);
            velocity = {0, 0};
            currentCenterY = window->getSize().y / 2.f;
            currentStateDuration = std::uniform_real_distribution<float>(5.f, 7.f)(rng);
            std::cout << "MageOrc: Entering FLYING state for " << currentStateDuration << "s.\n";
        } else if (choice <= 7) { // barrage
            currentState = State::BARRAGE_PREPARE;
            setAnimation("barrage", 4, 0.2f);
            velocity = {0, 0};
            currentStateDuration = 1.5f;
        } else { // flurry
            currentState = State::FLURRY;
            setAnimation("flurry", 5, 0.1f);
            velocity = {0, 0};
            currentStateDuration = std::uniform_real_distribution<float>(6.0f, 10.0f)(rng);
            std::cout << "MageOrc: Entering FLURRY state for " << currentStateDuration << "s." << std::endl;
        }
        stateTimer.restart();
    }

    void updateSinusoidalMovement(float centerY, float amplitude, float frequency) {
        float timeInState = stateTimer.getElapsedTime().asSeconds();
        float newY = centerY + amplitude * std::sin(timeInState * frequency * 2.0f * M_PI);

        float minY = 50.f;
        float maxY = window->getSize().y - 50.f;
        newY = std::max(minY, std::min(maxY, newY));

        setPosition(getPosition().x, newY);
    }

    sf::Vector2f calculateVagueAimDirection() {
        if (!playerPositionPtr) {
            return sf::Vector2f(-1.0f, 0.0f);
        }

        sf::Vector2f magePos = getPosition();
        sf::Vector2f playerPos = *playerPositionPtr;
        sf::Vector2f exactDir = playerPos - magePos;

        float exactAngle = std::atan2(exactDir.y, exactDir.x);

        std::uniform_real_distribution<float> varianceDist(-flurryAimVariance, flurryAimVariance);
        float variedAngle = exactAngle + varianceDist(rng);

        sf::Vector2f vagueDir(std::cos(variedAngle), std::sin(variedAngle));
        return vagueDir;
    }

    void prepareBarrage() {
        projectilesToSpawn.clear();
        sf::Vector2f centerPos = getPosition();

        for (int i = 0; i < barrageCount; ++i) {
            float angle = (static_cast<float>(i) / barrageCount) * 2.0f * M_PI;
            float spawnX = centerPos.x + barrageRadius * std::cos(angle);
            float spawnY = centerPos.y + barrageRadius * std::sin(angle);

            sf::Vector2f direction(std::cos(angle), std::sin(angle));
            projectilesToSpawn.push_back({sf::Vector2f(spawnX, spawnY), direction, barrageProjectileSpeed});
        }
        std::cout << "MageOrc: Prepared " << barrageCount << " barrage projectiles." << std::endl;
    }

    void fireBarrage() {
        std::cout << "MageOrc: Firing barrage!" << std::endl;
        chooseNextState();
    }

    void fireFlurryShot(float dt) {
        timeSinceLastAction += dt;
        if (timeSinceLastAction >= flurryShotInterval) {
            timeSinceLastAction -= flurryShotInterval;

            sf::Vector2f spawnPos = getPosition();
            spawnPos.x -= 30.f * currentScaleX;

            sf::Vector2f direction = calculateVagueAimDirection();

            projectilesToSpawn.push_back({spawnPos, direction, flurryProjectileSpeed});
        }
    }

public:
    MageOrc(sf::RenderWindow* win, const sf::Vector2f& startPos)
        : Entity(win),
          currentState(State::IDLE),
          flyAmplitudeY(win->getSize().y * 0.4f),
          flyFrequencyY(0.2f),
          idleAmplitudeY(30.f),
          idleFrequencyY(0.5f),
          currentCenterY(startPos.y),
          rng(std::chrono::steady_clock::now().time_since_epoch().count())
    {
        if (!win) { throw ConfigurationError("MageOrc requires a valid RenderWindow pointer."); }

        this->frameWidth = 96;
        this->frameHeight = 96;
        this->healthPoints = 300;

        try {
            loadAnimationTexture("idle", "assets/enemies/mage/Idle.png");
            loadAnimationTexture("fly", "assets/enemies/mage/Walk.png");
            loadAnimationTexture("flurry", "assets/enemies/mage/Magic_1.png");
            loadAnimationTexture("barrage", "assets/enemies/mage/Attack_2.png");
            loadAnimationTexture("death", "assets/enemies/mage/Dead.png");

            setAnimation("idle", 5, 0.3f);
            setScale(-2.5f, 2.5f);
            setPosition(startPos);


            float hitboxWidthRatio = 0.5f;
            float hitboxHeightRatio = 0.7f;
            float hitboxWidth = static_cast<float>(this->frameWidth) * hitboxWidthRatio;
            float hitboxHeight = static_cast<float>(this->frameHeight) * hitboxHeightRatio;
            float hitboxOffsetX = static_cast<float>(this->frameWidth) * (1.f - hitboxWidthRatio) / 2.f;
            float hitboxOffsetY = static_cast<float>(this->frameHeight) * (1.f - hitboxHeightRatio);

            customHitbox = sf::FloatRect(hitboxOffsetX, hitboxOffsetY, hitboxWidth, hitboxHeight);

            hitboxShape.setSize(sf::Vector2f(hitboxWidth, hitboxHeight));
            hitboxShape.setFillColor(sf::Color(255, 0, 255, 100));
            hitboxShape.setOutlineColor(sf::Color(200, 0, 200, 200));
            hitboxShape.setOutlineThickness(1.0f);

            chooseNextState();

        } catch (const ResourceLoadError& e) {
            std::cerr << "FATAL ERROR during MageOrc construction (ResourceLoadError): " << e.what() << std::endl;
            throw GameError("Failed to initialize MageOrc resources.");
        } catch (const InvalidStateError& e) {
            std::cerr << "FATAL ERROR during MageOrc construction (InvalidStateError): " << e.what() << std::endl;
            throw GameError("Failed to configure MageOrc state.");
        }
    }

    void actions() override {}

    void update(float dt) override {
        if (!isAlive) {
            if (currentAnimationName == "death") {
                Entity::update(dt);
                if (animationClock.getElapsedTime().asSeconds() >= currentAnimationInterval * currentNumFrames) {
                    markedForRemoval = true;
                }
            }
            return;
        }

        switch (currentState) {
            case State::IDLE:
                updateSinusoidalMovement(currentCenterY, idleAmplitudeY, idleFrequencyY);
            break;
            case State::FLYING:
                updateSinusoidalMovement(currentCenterY, flyAmplitudeY, flyFrequencyY);
            break;
            case State::BARRAGE_PREPARE:
                if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
                    prepareBarrage();
                    currentState = State::BARRAGE_FIRE;
                    stateTimer.restart();
                    currentStateDuration = 0.1f;
                    setAnimation("barrage", 8, 0.1f);
                    std::cout << "MageOrc: Transitioning to BARRAGE_FIRE." << std::endl;
                }
            break;
            case State::BARRAGE_FIRE:
                if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
                    fireBarrage();
                }
            break;
            case State::FLURRY:
                fireFlurryShot(dt);
            updateSinusoidalMovement(currentCenterY, idleAmplitudeY * 0.5f, idleFrequencyY * 1.5f);
            break;
            case State::ARTILLERY:
                break;
        }

        if (currentState != State::BARRAGE_FIRE && stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
            chooseNextState();
        }

        Entity::update(dt);
    }

    void draw() override {
        if (window) {
            window->draw(sprite);
            if (showHitbox) {
                sf::FloatRect globalBounds = getCollisionBounds();
                hitboxShape.setPosition(globalBounds.left, globalBounds.top);
                hitboxShape.setSize({globalBounds.width, globalBounds.height});
                window->draw(hitboxShape);
            }
        }
    }

    void takeDamage() override {
        if (!isAlive) return;
        healthPoints--;
        if (healthPoints <= 0) {
            isAlive = false;
            std::cout << "MageOrc defeated!\n";
            setAnimation("death", 5, 3.f);
            velocity = {0, 0};
            currentState = State::IDLE;
        }
    }

    void markForRemoval() {
        if (!markedForRemoval) {
            markedForRemoval = true;
            isAlive = false;
        }
    }

    bool isMarkedForRemoval() const { return markedForRemoval; }

    std::vector<MagicProjectileSpawnInfo> getProjectilesToSpawn() {
        std::vector<MagicProjectileSpawnInfo> queue = std::move(projectilesToSpawn);
        projectilesToSpawn.clear();
        return queue;
    }

    void updatePlayerPosition(const sf::Vector2f* playerPos) {
        playerPositionPtr = const_cast<sf::Vector2f*>(playerPos);
    }

    sf::FloatRect getCollisionBounds() const {
        return sprite.getTransform().transformRect(customHitbox);
    }

};

#endif //MAGEORC_H