#ifndef BERSERKORC_H
#define BERSERKORC_H

#include <SFML/Graphics.hpp>
#include <random>
#include "Entity.h"

class BerserkOrc : public Entity {
    enum class State { IDLE, WALKING }; // state enumeration
    State currentState{State::IDLE}; // current state initially idle

    const float patrolRange{150.f}; // range of linear movement
    sf::Vector2f originPoint; // origin spawn point
    const float speed{2.f}; // movement speed
    bool isMovingRight{true}; // direction flag

    sf::Clock stateTimer; // state timer
    float currentStateDuration{0.f};
    std::mt19937 rng;

    sf::RectangleShape hitboxShape;
    sf::FloatRect customHitbox;
    bool markedForRemoval{false}; // trigger flag for deletion

    void chooseNextState();

public:
    BerserkOrc(sf::RenderWindow* win, const sf::Vector2f& startPos);

    // override base class functions
    void actions() override;
    void update() override;
    void draw() override;
    void takeDamage() override;

    sf::FloatRect getCollisionBounds() const;
    void markForRemoval();
    bool isMarkedForRemoval() const;
    ~BerserkOrc() override = default;
};

#endif // BERSERKORC_H