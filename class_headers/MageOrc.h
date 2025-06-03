#ifndef MAGEORC_H
#define MAGEORC_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "Entity.h"

// projectile data structure
struct MagicProjectileSpawnInfo {
    sf::Vector2f position; // projectile position
    sf::Vector2f direction; // projectile direction
    float speed;
};

class MageOrc : public Entity {
    // state enumeration
    enum class State { IDLE, FLYING, BARRAGE_PREPARE, BARRAGE_FIRE, FLURRY, ARTILLERY};
    State currentState{State::IDLE}; // current state initially idle

    // state variables
    float flyAmplitudeY;
    float flyFrequencyY;
    float idleAmplitudeY;
    float idleFrequencyY;
    float currentCenterY;

    // state timers
    sf::Clock stateTimer;
    float currentStateDuration;
    sf::Clock actionTimer;
    float timeSinceLastAction;
    std::mt19937 rng;

    // class constants
    const int barrageCount{12};
    const float barrageRadius{100.f};
    const float barrageProjectileSpeed{3.f};
    const float flurryShotInterval{0.3f};
    const float flurryProjectileSpeed{6.f};
    const float m_pi{3.14159};
    const float flurryAimVariance{0.45f};
    sf::Vector2f* playerPositionPtr{nullptr};

    // projectile vector
    std::vector<MagicProjectileSpawnInfo> projectilesToSpawn;

    // hitbox information
    sf::RectangleShape hitboxShape;
    sf::FloatRect customHitbox;

    // flags for entity deletion
    bool markedForRemoval{false};
    bool isAlive{true};

    // private functions for state handling
    void chooseNextState();
    void updateSinusoidalMovement(float centerY, float amplitude, float frequency);
    sf::Vector2f calculateVagueAimDirection();
    void prepareBarrage();
    void fireBarrage();
    void fireFlurryShot(float dt);

public:
    MageOrc(sf::RenderWindow* win, const sf::Vector2f& startPos);

    // override base class functions
    void actions() override;
    void update() override;
    void draw() override;
    void takeDamage() override;

    void updater(float dt);
    void markForRemoval();
    bool isMarkedForRemoval() const;
    std::vector<MagicProjectileSpawnInfo> getProjectilesToSpawn();
    void updatePlayerPosition(sf::Vector2f* playerPos);
    sf::FloatRect getCollisionBounds() const;
};

#endif //MAGEORC_H