#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Entity.h"
#include "Platform.h"
#include "Subject.h"

struct ProjectileSpawnInfo {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed{};
};

class Player : public Entity, public Subject {
    // numeric constants
    const float moveSpeed{4.8f};
    const float gravityForce{0.7f};
    const float jumpStrength{-18.0f};
    const float defaultGroundY{900.f};
    const int shootCooldownFrames{13};
    const float projectileMoveSpeed{15.0f};
    const float dropThroughSpeed{3.0f};

    // flags
    bool isJumping{false};
    bool canJump{true};
    bool onGround{false};
    bool isDropping{false};
    int currentShootCooldown{0};
    bool facingRight{true};
    bool isShooting{false};
    bool wantsToShootFlag{false};

    // hitbox
    sf::RectangleShape hitboxShape_debug;
    sf::FloatRect customHitbox_local;

    static bool instanceExists; // ensure init params are used only once
    Player(sf::RenderWindow* win,
           const std::string& initialAnimationName,
           const std::string& initialTexturePath,
           int initialNumFrames,
           float initialAnimationInterval,
           const sf::Vector2f& startPosition);

public:
    // delete copy constructor and assignment operators for singleton
    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    static Player& getInstance(
        sf::RenderWindow* win,
        const std::string& initialAnimationName = "",
        const std::string& initialTexturePath = "",
        int initialNumFrames = 0,
        float initialAnimationInterval = 0.f,
        const sf::Vector2f& startPosition = {0, 0});

    // overridden base class functions
    void draw() override;
    void actions() override;
    void update() override;
    void takeDamage() override;

    void updater(const std::vector<Platform>& platforms);
    sf::FloatRect getHitboxGlobalBounds() const;
    void jump();
    bool wantsToShootProjectile() const;
    ProjectileSpawnInfo getProjectileSpawnDetails();
    sf::FloatRect getCollisionBounds() const;
};

#endif // PLAYER_H