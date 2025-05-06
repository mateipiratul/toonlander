#ifndef BERSERKORC_H
#define BERSERKORC_H

#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <random>
#include <chrono>
#include "Entity.h"

class BerserkOrc : public Entity {
    enum class State { IDLE, WALKING };
    State currentState;

    float patrolRange;
    sf::Vector2f originPoint;
    float speed;
    bool isMovingRight;

    sf::Clock stateTimer;
    float currentStateDuration;
    std::mt19937 rng;

    sf::RectangleShape hitboxShape;
    sf::FloatRect customHitbox;

    bool markedForRemoval = false;

    void chooseNextState() {
        std::uniform_real_distribution<float> idleDurationDist(2.0f, 4.0f);
        std::uniform_real_distribution<float> walkDurationDist(3.5f, 6.0f);

        std::uniform_int_distribution<int> stateChoiceDist(1, 10);
        int choice = stateChoiceDist(rng); // weighed decision for action

        if (choice <= 7 || currentState == State::IDLE) { // more likely to walk
            currentState = State::WALKING;
            currentStateDuration = walkDurationDist(rng);
            setAnimation("walk", 6, 0.15f);

            float currentX = getPosition().x;
            float leftBoundary = originPoint.x - patrolRange;
            float rightBoundary = originPoint.x + patrolRange;

            if (currentX >= rightBoundary) {
                isMovingRight = false;
            } else if (currentX <= leftBoundary) {
                isMovingRight = true;
            }

            velocity.x = isMovingRight ? speed : -speed;
            sprite.setScale(isMovingRight ? currentScaleX : -currentScaleX, currentScaleY);

            std::cout << "Orc: Entering WALK state for " << currentStateDuration << "s. Dir: " << (isMovingRight?"Right":"Left") << std::endl;

        } else { // go idle
            currentState = State::IDLE;
            currentStateDuration = idleDurationDist(rng);
            setAnimation("idle", 4, 0.2f);
            velocity.x = 0;
            std::cout << "Orc: Entering IDLE state for " << currentStateDuration << "s." << std::endl;
        }

        stateTimer.restart();
    }

public:
    BerserkOrc(sf::RenderWindow* win, const sf::Vector2f& startPos, float range = 150.f, float moveSpeed = 2.0f)
        : Entity(win), // Initialize base first
          currentState(State::IDLE), // Start idle before choosing state
          patrolRange(range),
          originPoint(startPos),
          speed(moveSpeed),
          isMovingRight(true),
          rng(static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count()))
    {
        try {
            this->frameWidth = 96;
            this->frameHeight = 96;
            this->healthPoints = 40;

            sprite.setOrigin(static_cast<float>(this->frameWidth) / 2.0f, static_cast<float>(this->frameHeight) / 2.0f);

            loadAnimationTexture("idle", "assets/enemies/berserk/Idle.png");
            loadAnimationTexture("walk", "assets/enemies/berserk/Walk.png");
            loadAnimationTexture("death", "assets/enemies/berserk/Dead.png");

            setScale(2.0f, 2.0f);
            setPosition(startPos);

            chooseNextState();

            float hitboxWidthRatio = 0.4f;
            float hitboxHeightRatio = 0.6f;
            float hitboxWidth = static_cast<float>(this->frameWidth) * hitboxWidthRatio;
            float hitboxHeight = static_cast<float>(this->frameHeight) * hitboxHeightRatio;
            float hitboxOffsetX = static_cast<float>(this->frameWidth) * (1.0f - hitboxWidthRatio) / 2.0f;
            float hitboxOffsetY = static_cast<float>(this->frameHeight) * (1.0f - hitboxHeightRatio);
            customHitbox = sf::FloatRect(hitboxOffsetX, hitboxOffsetY, hitboxWidth, hitboxHeight);

            hitboxShape.setSize(sf::Vector2f(hitboxWidth, hitboxHeight));
            hitboxShape.setFillColor(sf::Color(255, 0, 0, 100));
            hitboxShape.setOutlineColor(sf::Color(200, 0, 0, 200));
            hitboxShape.setOutlineThickness(1.0f);

        } catch (const ResourceLoadError& e) {
            std::cerr << "FATAL ERROR during BerserkOrc construction (ResourceLoadError): " << e.what() << std::endl;
            throw GameError("Failed to initialize BerserkOrc resources.");
        } catch (const InvalidStateError& e) {
             std::cerr << "FATAL ERROR during BerserkOrc construction (InvalidStateError): " << e.what() << std::endl;
             throw GameError("Failed to configure BerserkOrc state during initialization.");
        } catch (const GameError& e) {
             std::cerr << "FATAL ERROR during BerserkOrc construction (GameError): " << e.what() << std::endl;
             throw;
        } catch (const std::exception& e) {
             std::cerr << "FATAL std::exception during BerserkOrc construction: " << e.what() << std::endl;
             throw GameError("Standard exception during BerserkOrc initialization.");
        } catch (...) {
             std::cerr << "FATAL unknown exception during BerserkOrc construction." << std::endl;
             throw GameError("Unknown exception during BerserkOrc initialization.");
        }
    }

    void actions() override {}

    void update() override {
        bool boundaryReached = false;

        if (currentState == State::WALKING) {
            float currentX = getPosition().x;
            float leftBoundary = originPoint.x - patrolRange;
            float rightBoundary = originPoint.x + patrolRange;

            float nextX = currentX + velocity.x;

            if (isMovingRight && nextX >= rightBoundary) {
                setPosition(rightBoundary, getPosition().y);
                velocity.x = 0;
                boundaryReached = true;
                std::cout << "Orc: Reached right boundary." << std::endl;
            } else if (!isMovingRight && nextX <= leftBoundary) {
                setPosition(leftBoundary, getPosition().y);
                velocity.x = 0;
                boundaryReached = true;
                 std::cout << "Orc: Reached left boundary." << std::endl;
            }

            if (!boundaryReached) {
                 sprite.move(velocity.x, 0.f);
            }
        } else { velocity.x = 0; }

        if (stateTimer.getElapsedTime().asSeconds() >= currentStateDuration || boundaryReached) {
            chooseNextState();
        }

        Entity::update();
    }

    void draw() override {
        if (window) {
            window->draw(sprite);
            sf::FloatRect globalBounds = getCollisionBounds(); // method
            hitboxShape.setPosition(globalBounds.left, globalBounds.top);
            hitboxShape.setSize({globalBounds.width, globalBounds.height});
            window->draw(hitboxShape);
        }
    }

    sf::FloatRect getCollisionBounds() const {
        return sprite.getTransform().transformRect(customHitbox);
    }

    void takeDamage() override {
        healthPoints--;
        std::cout << "Orc took damage. HP: " << healthPoints << std::endl;
        if (healthPoints <= 0 && !markedForRemoval) {
            markedForRemoval = true;
            setAnimation("death", 4, 0.2f);
            velocity = {0,0}; // stop moving
            std::cout << "Orc marked for removal (dead)." << std::endl;
        }
    }

    void markForRemoval() {
        if (!markedForRemoval) {
             markedForRemoval = true;
             std::cout << "Orc marked for removal (external)." << std::endl;
        }
    }

    bool isMarkedForRemoval() const {
        return markedForRemoval;
    }
};

#endif // BERSERKORC_H