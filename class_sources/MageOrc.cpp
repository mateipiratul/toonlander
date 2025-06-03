#include "../class_headers/MageOrc.h"
#include "../class_headers/Entity.h"       // Should be included via MageOrc.h
#include "../class_headers/GameExceptions.h" // For ResourceLoadError, InvalidStateError etc.
#include <SFML/Graphics.hpp> // Should be included via MageOrc.h
#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>     // For M_PI, std::sin, std::cos, std::atan2, std::sqrt, std::abs

#define M_PI 3.14

// Helper for linear interpolation (can be moved to a utility file if used elsewhere)
float lerp(float a, float b, float t) {
    t = std::max(0.0f, std::min(1.0f, t)); // Clamp t to [0, 1]
    return a + t * (b - a);
}

MageOrc::MageOrc(sf::RenderWindow* win, const sf::Vector2f &startPos) :
    Entity(win),
    flyAmplitudeY(static_cast<float>(win->getSize().y) * 0.25f),
    flyFrequencyY(0.4f),
    idleAmplitudeY(25.f),
    idleFrequencyY(0.6f),
    currentCenterY(startPos.y),
    // currentStateDuration set in chooseNextState
    // actionTimer default constructed
    timeSinceLastAction(0.f),
    rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()))
    // projectilesToSpawn default constructed
    // hitboxShape default constructed
    // customHitbox default constructed
    // stateTimer default constructed
{
    this->frameWidth = 96;
    this->frameHeight = 96;
    this->healthPoints = 300;

    try {
        loadAnimationTexture("idle", "assets/enemies/mage/Idle.png");
        loadAnimationTexture("fly", "assets/enemies/mage/Walk.png");
        loadAnimationTexture("flurry", "assets/enemies/mage/Magic_1.png");
        loadAnimationTexture("barrage", "assets/enemies/mage/Attack_2.png");
        loadAnimationTexture("death", "assets/enemies/mage/Dead.png");

        Entity::setScale(-2.5f, 2.5f);
        setPosition(startPos);

        float hitboxWidthRatio = 0.5f;
        float hitboxHeightRatio = 0.7f;
        float hitboxWidth_unscaled = static_cast<float>(this->frameWidth) * hitboxWidthRatio;
        float hitboxHeight_unscaled = static_cast<float>(this->frameHeight) * hitboxHeightRatio;
        float hitboxOffsetX_unscaled = static_cast<float>(this->frameWidth) * (1.f - hitboxWidthRatio) / 2.f;
        float hitboxOffsetY_unscaled = static_cast<float>(this->frameHeight) * (1.f - hitboxHeightRatio);
        customHitbox = sf::FloatRect(hitboxOffsetX_unscaled, hitboxOffsetY_unscaled, hitboxWidth_unscaled, hitboxHeight_unscaled);

        hitboxShape.setSize(sf::Vector2f(hitboxWidth_unscaled * this->currentScaleX, hitboxHeight_unscaled * this->currentScaleY));
        hitboxShape.setFillColor(sf::Color(255, 0, 255, 100));
        hitboxShape.setOutlineColor(sf::Color(200, 0, 200, 200));
        hitboxShape.setOutlineThickness(1.0f);

        chooseNextState();

    } catch (const ResourceLoadError& e) {
        std::cerr << "FATAL ERROR during MageOrc construction (ResourceLoadError): " << e.what() << std::endl;
        throw GameError("Failed to initialize MageOrc resources: " + std::string(e.what()));
    } catch (const InvalidStateError& e) {
        std::cerr << "FATAL ERROR during MageOrc construction (InvalidStateError): " << e.what() << std::endl;
        throw GameError("Failed to configure MageOrc state: " + std::string(e.what()));
    } catch (const std::exception& e) {
        std::cerr << "FATAL std::exception during MageOrc construction: " << e.what() << std::endl;
        throw GameError("Standard exception during MageOrc initialization: " + std::string(e.what()));
    } catch (...) {
        std::cerr << "FATAL unknown exception during MageOrc construction." << std::endl;
        throw GameError("Unknown exception during MageOrc initialization.");
    }
}

void MageOrc::chooseNextState() {
    if (!isAlive) { // If dead, try to set death animation if not already set
        if (currentAnimationName != "death") {
            // Assuming "death" animation has 5 frames based on your comment
            setAnimation("death", 5, 0.25f); // 0.25s per frame = 1.25s total
        }
        currentState = State::IDLE; // Or a specific State::DEAD
        velocity = {0, 0};
        currentStateDuration = 9999.f; // Effectively infinite, waits for removal
        stateTimer.restart();
        return;
    }

    std::uniform_int_distribution<int> stateChoiceDist(1,10);
    int choice = stateChoiceDist(rng);

    actionTimer.restart();
    timeSinceLastAction = 0.f;
    projectilesToSpawn.clear();

    // Define animation parameters here for clarity
    const int idleFrames = 4;    float idleInterval = 0.2f;
    const int flyFrames = 4;     float flyInterval = 0.15f;
    const int barrageFrames = 4; // Assuming barrage animation covers prepare & fire, or use distinct ones
                                 // Your old code used setAnimation("barrage", 4, 0.2f) for prepare
                                 // then setAnimation("barrage", 8, 0.1f) for fire.
                                 // Let's assume a single "barrage" animation that plays through.
                                 // If it has distinct parts, you'd need two animation names.
                                 // For now, "barrage" with more frames if it's a combined anim.
    float barrageInterval = 0.15f; // Adjust if barrage anim is longer
    const int flurryFrames = 5;  float flurryInterval = 0.12f;


    if (choice <= 1) { // IDLE
        currentState = State::IDLE;
        setAnimation("idle", idleFrames, idleInterval);
        velocity = {0, 0};
        currentCenterY = getPosition().y;
        currentStateDuration = std::uniform_real_distribution<float>(2.f, 3.f)(rng);
        // std::cout << "MageOrc: IDLE. CenterY: " << currentCenterY << std::endl;
    } else if (choice <= 3) { // FLYING
        currentState = State::FLYING;
        setAnimation("fly", flyFrames, flyInterval);
        velocity = {0, 0};
        float targetFlyCenterY = static_cast<float>(window->getSize().y) / 2.f + std::uniform_real_distribution<float>(-flyAmplitudeY * 0.3f, flyAmplitudeY * 0.3f)(rng);
        currentCenterY = targetFlyCenterY;
        currentStateDuration = std::uniform_real_distribution<float>(4.f, 6.f)(rng);
        // std::cout << "MageOrc: FLYING. Target CenterY: " << currentCenterY << std::endl;
    } else if (choice <= 6) { // BARRAGE
        currentState = State::BARRAGE_PREPARE;
        setAnimation("barrage", barrageFrames, barrageInterval); // Use a single barrage animation for now
        velocity = {0, 0};
        currentCenterY = getPosition().y;
        currentStateDuration = 1.0f; // Duration of BARRAGE_PREPARE state
        // std::cout << "MageOrc: BARRAGE_PREPARE. CenterY: " << currentCenterY << std::endl;
    } else { // FLURRY
        currentState = State::FLURRY;
        setAnimation("flurry", flurryFrames, flurryInterval);
        velocity = {0, 0};
        currentCenterY = getPosition().y;
        currentStateDuration = std::uniform_real_distribution<float>(3.0f, 5.0f)(rng);
        // std::cout << "MageOrc: FLURRY. CenterY: " << currentCenterY << std::endl;
    }
    stateTimer.restart();
}

void MageOrc::updateSinusoidalMovement(float centerY, float amplitude, float frequency) {
    if (!isAlive) return;
    float timeForSine = actionTimer.getElapsedTime().asSeconds();
    float verticalOffset = amplitude * std::sin(timeForSine * frequency * 2.0f * M_PI);
    float newY = centerY + verticalOffset;
    setPosition(getPosition().x, newY);
}

sf::Vector2f MageOrc::calculateVagueAimDirection() {
    if (!isAlive) return (sprite.getScale().x < 0.0f) ? sf::Vector2f(-1.0f, 0.0f) : sf::Vector2f(1.0f, 0.0f);

    if (!playerPositionPtr) {
        return (sprite.getScale().x < 0.0f) ? sf::Vector2f(-1.0f, 0.0f) : sf::Vector2f(1.0f, 0.0f);
    }

    sf::Vector2f magePos = getPosition();
    sf::Vector2f playerPosVal = *playerPositionPtr; // Dereference (it's sf::Vector2f* now)
    sf::Vector2f exactDir = playerPosVal - magePos;

    float magSq = exactDir.x * exactDir.x + exactDir.y * exactDir.y;
    constexpr float epsilonSq = 0.1f * 0.1f;

    if (magSq < epsilonSq) {
        return (sprite.getScale().x < 0.0f) ? sf::Vector2f(-1.0f, 0.0f) : sf::Vector2f(1.0f, 0.0f);
    }

    float exactAngle = std::atan2(exactDir.y, exactDir.x);
    std::uniform_real_distribution<float> varianceDist(-flurryAimVariance, flurryAimVariance);
    float variedAngle = exactAngle + varianceDist(rng);

    return {std::cos(variedAngle), std::sin(variedAngle)};
}

void MageOrc::prepareBarrage() {
    if (!isAlive) return;
    projectilesToSpawn.clear();
    sf::Vector2f centerPos = getPosition();
    for (int i = 0; i < barrageCount; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(barrageCount)) * 2.0f * M_PI;
        float spawnRadius = 30.f;
        float spawnX = centerPos.x + spawnRadius * std::cos(angle);
        float spawnY = centerPos.y + spawnRadius * std::sin(angle);
        sf::Vector2f direction(std::cos(angle), std::sin(angle));
        projectilesToSpawn.push_back({{spawnX, spawnY}, direction, barrageProjectileSpeed});
    }
}

void MageOrc::fireBarrage() {
    if (!isAlive) return;
    chooseNextState();
}

void MageOrc::fireFlurryShot(float dt) {
    if (!isAlive) return;
    timeSinceLastAction += dt;
    if (timeSinceLastAction >= flurryShotInterval) {
        timeSinceLastAction = 0.f;
        sf::Vector2f spawnPos = getPosition();
        float horizontalOffset = 40.f;
        // Adjust x based on which way sprite is scaled (facing)
        // Entity::currentScaleX stores absolute scale. sf::Sprite::getScale().x has the sign.
        spawnPos.x += (sprite.getScale().x > 0 ? horizontalOffset : -horizontalOffset);
        sf::Vector2f direction = calculateVagueAimDirection();
        projectilesToSpawn.push_back({spawnPos, direction, flurryProjectileSpeed});
    }
}

void MageOrc::actions() { /* AI logic is in updater */ }
void MageOrc::update() { /* Entity::update() is called by updater() */ }

void MageOrc::updater(float dt) {
    // Logic for when !isAlive (death animation handling)
    if (!isAlive) {
        if (currentAnimationName == "death") {
            // Manual check for death animation completion since isAnimationFinished was removed from Entity
            // Assumes "death" animation is 5 frames, and currentAnimationInterval is set correctly for it.
            const int deathAnimTotalFrames = 5; // Match what's in takeDamage()
            if (this->currentFrameIndex == (deathAnimTotalFrames - 1) &&
                this->animationClock.getElapsedTime().asSeconds() >= this->currentAnimationInterval) {
                if (!markedForRemoval) {
                    // std::cout << "MageOrc: Death animation cycle complete, marking for removal." << std::endl;
                    markedForRemoval = true;
                }
            }
        }
        Entity::update(); // Continue updating animation (e.g., playing out death)
        return;
    }

    // Current state logic
    switch (currentState) {
        case State::IDLE:
            updateSinusoidalMovement(currentCenterY, idleAmplitudeY, idleFrequencyY);
            break;
        case State::FLYING:
            updateSinusoidalMovement(currentCenterY, flyAmplitudeY, flyFrequencyY);
            break;
        case State::BARRAGE_PREPARE:
            updateSinusoidalMovement(currentCenterY, 10.f, 0.2f);
            if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
                prepareBarrage();
                currentState = State::BARRAGE_FIRE;
                stateTimer.restart();
                currentStateDuration = 0.5f; // Duration of "firing" pose
                // If "barrage" animation is long and covers both prepare and fire,
                // you might not need to call setAnimation again here.
                // If they are distinct, you would: setAnimation("barrage_fire", ...);
                // For now, assuming "barrage" animation continues or you manage frames.
                // std::cout << "MageOrc: Transitioning to BARRAGE_FIRE." << std::endl;
            }
            break;
        case State::BARRAGE_FIRE:
            if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
                fireBarrage(); // This calls chooseNextState
            }
            break;
        case State::FLURRY:
            fireFlurryShot(dt);
            updateSinusoidalMovement(currentCenterY, idleAmplitudeY * 0.7f, idleFrequencyY * 1.2f);
            break;
        case State::ARTILLERY: // Not implemented yet
            updateSinusoidalMovement(currentCenterY, idleAmplitudeY, idleFrequencyY);
            break;
    }

    // Check for overall state duration timeout
    if (currentState != State::BARRAGE_FIRE && currentState != State::BARRAGE_PREPARE) {
         if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration) {
            chooseNextState();
        }
    }
    Entity::update(); // Update current animation frames
}

void MageOrc::draw() {
    if (window) {
        window->draw(sprite);
        if (isAlive) {
            sf::FloatRect globalBounds = getCollisionBounds();
            hitboxShape.setPosition(globalBounds.left, globalBounds.top);
            hitboxShape.setSize({globalBounds.width, globalBounds.height});
            window->draw(hitboxShape);
        }
    }
}

void MageOrc::takeDamage() {
    if (!isAlive || markedForRemoval) return;

    healthPoints--;
    // std::cout << "MageOrc took damage. HP: " << healthPoints << std::endl;

    if (healthPoints <= 0) {
        isAlive = false;
        // std::cout << "MageOrc defeated!" << std::endl;
        setAnimation("death", 5, 0.25f); // 5 frames, 0.25s interval per frame
        velocity = {0, 0};
        currentState = State::IDLE; // Or a State::DEAD
        currentStateDuration = 9999.f;
        stateTimer.restart();
        projectilesToSpawn.clear();
    }
}

void MageOrc::markForRemoval() {
    if (!markedForRemoval) {
        markedForRemoval = true;
        isAlive = false;
    }
}

bool MageOrc::isMarkedForRemoval() const {
    return markedForRemoval;
}

std::vector<MagicProjectileSpawnInfo> MageOrc::getProjectilesToSpawn() {
    std::vector<MagicProjectileSpawnInfo> queueToReturn = std::move(projectilesToSpawn);
    projectilesToSpawn.clear();
    return queueToReturn;
}

// Corrected signature to match your MageOrc.h (sf::Vector2f*)
void MageOrc::updatePlayerPosition(sf::Vector2f* playerPos) {
    playerPositionPtr = playerPos;
}

sf::FloatRect MageOrc::getCollisionBounds() const {
    return sprite.getTransform().transformRect(customHitbox);
}