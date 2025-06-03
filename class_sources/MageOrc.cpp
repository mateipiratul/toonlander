#include "../class_headers/MageOrc.h"
#include "../class_headers/Entity.h"
#include "../class_headers/GameExceptions.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

void MageOrc::chooseNextState() {
    if (!isAlive) {
        currentState = State::IDLE;
        velocity = {0, 0};
        return;
    }

    std::uniform_int_distribution stateChoiceDist(1,10);
    int choice = stateChoiceDist(rng); // weighted choice

    actionTimer.restart();
    timeSinceLastAction = 0.f;

    if (choice == 1) { // idle
        currentState = State::IDLE;
        setAnimation("idle", 4, 0.2f);
        velocity = {0, 0};
        currentCenterY = getPosition().y;
        currentStateDuration = std::uniform_real_distribution(2.f, 3.f)(rng);
        std::cout << "MageOrc: Entering IDLE state for " << currentStateDuration << " s.\n";
    } else if (choice <= 3) { // flying
        currentState = State::FLYING;
        setAnimation("fly", 4, 0.2f);
        velocity = {0, 0};
        currentCenterY = static_cast<float>(window->getSize().y) / 2.f;
        currentStateDuration = std::uniform_real_distribution(5.f, 7.f)(rng);
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

void MageOrc::updateSinusoidalMovement(float centerY, float amplitude, float frequency) {
    float timeInState = stateTimer.getElapsedTime().asSeconds();
    float newY = centerY + amplitude * std::sin(timeInState * frequency * 2.0f * m_pi);

    setPosition(getPosition().x, newY);
}

sf::Vector2f MageOrc::calculateVagueAimDirection() {
    if (!playerPositionPtr) {
        return sf::Vector2f{-1.0f, 0.0f};
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

void MageOrc::prepareBarrage() {
    projectilesToSpawn.clear();
    sf::Vector2f centerPos = getPosition();

    for (int i = 0; i < barrageCount; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(barrageCount)) * 2.0f * m_pi;
        float spawnX = centerPos.x + barrageRadius * std::cos(angle);
        float spawnY = centerPos.y + barrageRadius * std::sin(angle);

        sf::Vector2f direction(std::cos(angle), std::sin(angle));
        projectilesToSpawn.push_back({sf::Vector2f(spawnX, spawnY), direction, barrageProjectileSpeed});
    }

    std::cout << "MageOrc: Prepared " << barrageCount << " barrage projectiles." << std::endl;
}

void MageOrc::fireBarrage() {
    std::cout << "MageOrc: Firing barrage!" << std::endl;
    chooseNextState();
}

void MageOrc::fireFlurryShot(float dt) {
    timeSinceLastAction += dt;
    if (timeSinceLastAction >= flurryShotInterval) {
        timeSinceLastAction -= flurryShotInterval;

        sf::Vector2f spawnPos = getPosition();
        spawnPos.x -= 30.f * currentScaleX;

        sf::Vector2f direction = calculateVagueAimDirection();

        projectilesToSpawn.push_back({spawnPos, direction, flurryProjectileSpeed});
    }
}

MageOrc::MageOrc(sf::RenderWindow* win, const sf::Vector2f &startPos) :
    Entity(win),
    flyAmplitudeY(static_cast<float>(win->getSize().y) * 0.4f),
    currentCenterY(startPos.y),
    rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count())) {
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

void MageOrc::actions() {}
void MageOrc::update() {}

void MageOrc::updater(float dt) {
    if (!isAlive) {
        if (currentAnimationName == "death") {
            markedForRemoval = true;
        }
        Entity::update();
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

    Entity::update();
}

void MageOrc::draw() {
    if (window) {
        window->draw(sprite);
        sf::FloatRect globalBounds = getCollisionBounds();
        hitboxShape.setPosition(globalBounds.left, globalBounds.top);
        hitboxShape.setSize({globalBounds.width, globalBounds.height});
        window->draw(hitboxShape);
    }
}

void MageOrc::takeDamage() {
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

void MageOrc::markForRemoval() {
    if (!markedForRemoval) {
        markedForRemoval = true;
        isAlive = false;
    }
}

bool MageOrc::isMarkedForRemoval() const { return markedForRemoval; }

std::vector<MagicProjectileSpawnInfo> MageOrc::getProjectilesToSpawn() {
    std::vector<MagicProjectileSpawnInfo> queue = std::move(projectilesToSpawn);
    projectilesToSpawn.clear();
    return queue;
}

void MageOrc::updatePlayerPosition(sf::Vector2f* playerPos) {
    playerPositionPtr = playerPos;
}

sf::FloatRect MageOrc::getCollisionBounds() const {
    return sprite.getTransform().transformRect(customHitbox);
}