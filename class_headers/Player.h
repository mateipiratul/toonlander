#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "Entity.h"
#include "Platform.h"
#include "GameExceptions.h"

struct ProjectileSpawnInfo {
    sf::Vector2f position;
    sf::Vector2f direction;
    float speed{};
};

class Player : public Entity {
    const float speed = 4.8f;
    const float gravity = 0.7f;
    const float jumpForce = -18.0f;
    const float groundY = 900.f;
    const int shootCooldownTime = 13;
    const float projectileSpeed = 15.0f;
    const float dropThroughVelocity = 3.0f;

    bool isJumping = false;
    bool canJump = true;
    bool onGround = false;
    bool isDropping = false;
    int currentShootCooldown = 0;
    bool facingRight = true;
    bool isShooting = false;
    bool wantsToShoot = false;

    sf::RectangleShape hitboxShape;
    sf::FloatRect customHitbox;

public:
    Player(sf::RenderWindow* win,
       const std::string& initialAnimationName,
       const std::string& initialTexturePath,
       int initialNumFrames,
       float initialAnimationInterval,
       const sf::Vector2f& startPosition)
    : Entity(win)
    {
        try {
            this->frameWidth = 128;
            this->frameHeight = 128;
            this->healthPoints = 3;

            loadAnimationTexture(initialAnimationName, initialTexturePath); // idle
            loadAnimationTexture("run", "assets/player/Run.png");
            loadAnimationTexture("jump", "assets/player/Jump.png");
            loadAnimationTexture("shoot", "assets/player/Shot.png");
            loadAnimationTexture("hurt", "assets/player/Hurt.png");
            loadAnimationTexture("death", "assets/player/Dead.png");

            setAnimation(initialAnimationName, initialNumFrames, initialAnimationInterval);

            setScale(2.0f, 2.0f);
            setPosition(startPosition);
            facingRight = true;

            float hitboxWidth = 30.f;
            float hitboxHeight = static_cast<float>(this->frameHeight) * (3.f / 5.f);
            float hitboxOffsetX = (static_cast<float>(this->frameWidth) - hitboxWidth) / 2.0f;
            float hitboxOffsetY = static_cast<float>(this->frameHeight) - hitboxHeight;
            customHitbox = sf::FloatRect(hitboxOffsetX, hitboxOffsetY, hitboxWidth, hitboxHeight);
            sprite.setOrigin(static_cast<float>(this->frameWidth) / 2.f, static_cast<float>(this->frameHeight) / 2.f);

            hitboxShape.setSize(sf::Vector2f(hitboxWidth, hitboxHeight));
            hitboxShape.setFillColor(sf::Color(0, 255, 0, 100));
            hitboxShape.setOutlineColor(sf::Color::Green);
            hitboxShape.setOutlineThickness(1.0f);

        } catch (const ResourceLoadError& e) {
            std::cerr << "FATAL ERROR during Player construction (ResourceLoadError): " << e.what() << std::endl;
            throw GameError("Failed to initialize Player resources due to loading error.");

        } catch (const InvalidStateError& e) {
            std::cerr << "FATAL ERROR during Player construction (InvalidStateError): " << e.what() << std::endl;
            throw GameError("Failed to configure Player state during initialization.");

        } catch (const GameError& e) {
            std::cerr << "FATAL ERROR during Player construction (GameError): " << e.what() << std::endl;
            throw;
        }
    }

    sf::FloatRect getHitboxGlobalBounds() const {
        sf::Transform transform = sprite.getTransform();
        return transform.transformRect(customHitbox);
    }

    void draw() override {
        if (window) {
            window->draw(this->sprite);
            sf::FloatRect globalHitbox = getHitboxGlobalBounds();
            hitboxShape.setPosition(globalHitbox.left, globalHitbox.top);
            hitboxShape.setSize({globalHitbox.width, globalHitbox.height});
            window->draw(hitboxShape);
        }
    }

    void actions() override {
        wantsToShoot = false;
        bool tryingToShoot = sf::Keyboard::isKeyPressed(sf::Keyboard::X);

            if (tryingToShoot && currentShootCooldown <= 0 && onGround && velocity.x == 0 && !isJumping && !isShooting) {
                wantsToShoot = true;
                setAnimation("shoot", 4, 0.09f);
                sprite.setOrigin(static_cast<float>(this->frameWidth) / 2.0f, static_cast<float>(this->frameHeight) / 2.0f);
                isShooting = true;
                currentShootCooldown = shootCooldownTime;
            }

        if (currentShootCooldown > 0) {
            currentShootCooldown--;
        }

        if (!isShooting) {
            bool movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
            bool movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

            if (movingLeft && getPosition().x > getHitboxGlobalBounds().width / 2.0f) {
                velocity.x = -speed;
                facingRight = false;
                sprite.setScale(-std::abs(currentScaleX), currentScaleY);

                if (onGround && !isJumping) {
                    setAnimation("run", 10, 0.05f);
                }
            } else if (movingRight && getPosition().x < static_cast<float>(window->getSize().x) - getHitboxGlobalBounds().width / 2.0f) {
                velocity.x = speed;
                facingRight = true;
                sprite.setScale(std::abs(currentScaleX), currentScaleY);

                if (onGround && !isJumping) {
                    setAnimation("run", 10, 0.05f);
                }
            } else {
                velocity.x = 0;
                if (onGround && !isJumping) {
                    setAnimation("idle", 6, 0.1f);
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                if (canJump && onGround) {
                    jump();
                }
            } else {
                canJump = true;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::C) && onGround && !isJumping) {
                isDropping = true;
                onGround = false;
                velocity.y = dropThroughVelocity;
            }
        }
    }

    // void startMovingLeft() {
    //     if (!isJumping && !isShooting) {
    //         setAnimation("run", 10, 0.05f);
    //         velocity.x = -speed;
    //         facingRight = false;
    //         sprite.setScale(-std::abs(currentScaleX), currentScaleY); // Use abs for safety
    //     }
    // }

    // void startMovingRight() {
    //     if (!isJumping && !isShooting) {
    //         setAnimation("run", 10, 0.05f);
    //         velocity.x = speed;
    //         facingRight = true;
    //         sprite.setScale(std::abs(currentScaleX), currentScaleY);
    //     }
    // }

    // void stopMoving() {
    //     if (!isJumping && velocity.x != 0 && !isShooting) { setAnimation("idle", 6, 0.1f); }
    //     if (!isShooting) { velocity.x = 0; }
    // }

    void jump() {
        if (onGround && !isShooting) {
            setAnimation("jump", 10, 0.05f);
            velocity.y = jumpForce;
            isJumping = true;
            onGround = false;
            canJump = false;
            isDropping = false;
        }
    }

    void update() override {}

    void updater(const std::vector<Platform>& platforms) {
        if (isShooting && currentAnimationName == "shoot") {
             if (currentShootCooldown < 5) {
                 isShooting = false;
                 if (onGround && velocity.x == 0) {
                     setAnimation("idle", 6, 0.1f);
                 }
             }
        }

        if (!onGround) { velocity.y += gravity; }
        sprite.move(isShooting ? 0.f : velocity.x, velocity.y);

        onGround = false;
        sf::FloatRect playerHitbox = getHitboxGlobalBounds();
        sf::Vector2f currentPosition = getPosition();

        float groundCheckY = groundY;
        if (playerHitbox.top + playerHitbox.height >= groundCheckY && velocity.y >= 0) {
             float targetY = groundCheckY - (customHitbox.top + customHitbox.height - static_cast<float>(frameHeight) / 2.0f) * currentScaleY;
             setPosition(currentPosition.x, targetY);
             velocity.y = 0;
             isJumping = false;
             onGround = true;
             isDropping = false;
             if (!isShooting && velocity.x == 0 && currentAnimationName != "idle") {
                  setAnimation("idle", 6, 0.1f);
             }
        }

        for (const auto& platform : platforms) {
            sf::FloatRect platformBounds = platform.getBounds();
             playerHitbox = getHitboxGlobalBounds();

            if (playerHitbox.intersects(platformBounds)) {
                 float landingTolerance = 5.0f;

                if (velocity.y >= 0 && !isDropping &&
                    (playerHitbox.top + playerHitbox.height) >= platformBounds.top &&
                    (playerHitbox.top + playerHitbox.height) <= platformBounds.top + platformBounds.height + landingTolerance) // Ensure bottom is within platform bounds vertically
                {
                    if (playerHitbox.left < platformBounds.left + platformBounds.width &&
                        playerHitbox.left + playerHitbox.width > platformBounds.left)
                    {
                        float targetY = platformBounds.top - (customHitbox.top + customHitbox.height - static_cast<float>(frameHeight) / 2.0f) * currentScaleY;
                        setPosition(currentPosition.x, targetY);
                        velocity.y = 0;
                        isJumping = false;
                        onGround = true;
                        isDropping = false;
                        if (!isShooting && velocity.x == 0 && currentAnimationName != "idle") {
                            setAnimation("idle", 6, 0.1f);
                        }
                        break;
                    }
                }
            }
        }

        Entity::update();
    }

    bool wantsToShootProjectile() const { return wantsToShoot; }

    ProjectileSpawnInfo getProjectileSpawnDetails() {
        sf::Vector2f spawnPos = getPosition();
        spawnPos.x += facingRight ? (getHitboxGlobalBounds().width - 10.f) : -(getHitboxGlobalBounds().width - 10.f);
        spawnPos.y += static_cast<float>(frameHeight) / 2.0f - 15.f;
        sf::Vector2f direction = facingRight ? sf::Vector2f(1.f, 0.f) : sf::Vector2f(-1.f, 0.f);

        wantsToShoot = false;

        return {spawnPos, direction, projectileSpeed};
    }

    void takeDamage() override {
        healthPoints--;
        setAnimation("hurt", 5, 0.1f);
        // temporary invincibility to be implemented (2-3 seconds)
        if (healthPoints <= 0) {
            setAnimation("death", 5, 0.25f);
            velocity = {0,0}; // stop moving
        }
    }

    sf::FloatRect getCollisionBounds() const {
        return getHitboxGlobalBounds();
    }
};

#endif // PLAYER_H