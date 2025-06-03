#include "../class_headers/Player.h"
#include "../class_headers/Entity.h"
#include "../class_headers/Platform.h"
#include "../class_headers/GameExceptions.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>

bool Player::instanceExists = false;

Player& Player::getInstance(sf::RenderWindow* win,
                            const std::string& initialAnimationName,
                            const std::string& initialTexturePath,
                            int initialNumFrames,
                            float initialAnimationInterval,
                            const sf::Vector2f& startPosition) {
    if (!instanceExists && !win) {
        throw std::runtime_error("Player::getInstance() called for the first time with nullptr window. Initialization required.");
    }

    static Player instance(win, initialAnimationName, initialTexturePath,
                           initialNumFrames, initialAnimationInterval, startPosition);

    return instance;
}

Player::Player(sf::RenderWindow* win,
               const std::string& initialAnimationName,
               const std::string& initialTexturePath,
               int initialNumFrames,
               float initialAnimationInterval,
               const sf::Vector2f& startPosition)
    : Entity(win) {
    if (instanceExists) { throw std::runtime_error("Player singleton already constructed. Do not call constructor directly."); }

    if (!win) { throw GameLogicError("Player constructor called with nullptr window (should be via getInstance)."); }

    try {
        this->frameWidth = 128;
        this->frameHeight = 128;
        this->healthPoints = 3;

        loadAnimationTexture(initialAnimationName, initialTexturePath);
        loadAnimationTexture("run", "assets/player/Run.png");
        loadAnimationTexture("jump", "assets/player/Jump.png");
        loadAnimationTexture("shoot", "assets/player/Shot.png");
        loadAnimationTexture("hurt", "assets/player/Hurt.png");
        loadAnimationTexture("death", "assets/player/Dead.png");

        Entity::setScale(2.0f, 2.0f);
        setPosition(startPosition);
        facingRight = true;
        Entity::setAnimation(initialAnimationName, initialNumFrames, initialAnimationInterval);

        float hitboxWidth_unscaled = 30.f;
        float hitboxHeight_unscaled = static_cast<float>(this->frameHeight) * (3.f / 5.f);
        float hitboxOffsetX_local_unscaled = (static_cast<float>(this->frameWidth) - hitboxWidth_unscaled) / 2.0f;
        float hitboxOffsetY_local_unscaled = static_cast<float>(this->frameHeight) - hitboxHeight_unscaled;
        customHitbox_local = sf::FloatRect(hitboxOffsetX_local_unscaled, hitboxOffsetY_local_unscaled, hitboxWidth_unscaled, hitboxHeight_unscaled);

        // sprite.setOrigin(static_cast<float>(this->frameWidth) / 2.f, static_cast<float>(this->frameHeight) / 2.f);

        hitboxShape_debug.setSize(sf::Vector2f(customHitbox_local.width * currentScaleX, customHitbox_local.height * currentScaleY));
        hitboxShape_debug.setFillColor(sf::Color(0, 255, 0, 100));
        hitboxShape_debug.setOutlineColor(sf::Color::Green);
        hitboxShape_debug.setOutlineThickness(1.0f);

        instanceExists = true; // mark that the instance has been constructed

    } catch (const ResourceLoadError& e) {
        std::cerr << "FATAL ERROR during Player construction (ResourceLoadError): " << e.what() << std::endl;
        instanceExists = false; // construction failed
        throw GameError("Failed to initialize Player resources due to loading error.");
    } catch (const InvalidStateError& e) {
        std::cerr << "FATAL ERROR during Player construction (InvalidStateError): " << e.what() << std::endl;
        instanceExists = false;
        throw GameError("Failed to configure Player state during initialization.");
    } catch (const GameError& e) {
        std::cerr << "FATAL ERROR during Player construction (GameError): " << e.what() << std::endl;
        instanceExists = false;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "FATAL std::exception during Player construction: " << e.what() << std::endl;
        instanceExists = false;
        throw GameError("Standard exception during Player initialization.");
    } catch (...) {
        std::cerr << "FATAL unknown exception during Player construction." << std::endl;
        instanceExists = false;
        throw GameError("Unknown exception during Player initialization.");
    }
}

sf::FloatRect Player::getHitboxGlobalBounds() const { return sprite.getTransform().transformRect(customHitbox_local); }

sf::FloatRect Player::getCollisionBounds() const { return getHitboxGlobalBounds(); }

void Player::draw() {
    if (window) {
        window->draw(this->sprite);
        sf::FloatRect globalHitbox = getHitboxGlobalBounds();
        hitboxShape_debug.setPosition(globalHitbox.left, globalHitbox.top);
        hitboxShape_debug.setSize({globalHitbox.width, globalHitbox.height});
        window->draw(hitboxShape_debug);
    }
}

void Player::actions() {
    // if (healthPoints <= 0) { // no actions if dead
    //     velocity.x = 0; // ensure stopped
    //     return;
    // }

    wantsToShootFlag = false;
    bool tryingToShoot = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
    if (tryingToShoot && currentShootCooldown <= 0 && onGround && velocity.x == 0 && !isJumping && !isShooting) {
        wantsToShootFlag = true;
        setAnimation("shoot", 4, 0.09f);
        isShooting = true;
        currentShootCooldown = shootCooldownFrames;
    }

    if (currentShootCooldown > 0) { currentShootCooldown--; }

    if (!isShooting) {
        bool movingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        bool movingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

        if (movingLeft) {
            velocity.x = -moveSpeed;
            facingRight = false;
        } else if (movingRight) {
            velocity.x = moveSpeed;
            facingRight = true;
        } else { velocity.x = 0; }

        sprite.setScale(facingRight ? this->currentScaleX : -this->currentScaleX, this->currentScaleY);

        if (onGround && !isJumping) {
            if (velocity.x != 0) {
                if (currentAnimationName != "run") setAnimation("run", 10, 0.05f);
            } else if (currentAnimationName != "idle") setAnimation("idle", 6, 0.1f);
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
            velocity.y = dropThroughSpeed;
        }
    }
}

void Player::jump() {
    setAnimation("jump", 10, 0.05f);
    notifyObservers(GameEvent::PLAYER_JUMPED);
    velocity.y = jumpStrength;
    isJumping = true;
    onGround = false;
    canJump = false;
    isDropping = false;
}

void Player::update() { Entity::update(); }

void Player::updater(const std::vector<Platform>& platforms) {
    if (healthPoints <= 0) { // If dead, only physics might apply if falling during death anim
        if (!onGround) velocity.y += gravityForce;
        sprite.move(0, velocity.y); // Only vertical movement if any
        // Simplified ground check if falling while dead
        sf::FloatRect playerHitbox = getHitboxGlobalBounds();
        if (playerHitbox.top + playerHitbox.height >= defaultGroundY && velocity.y >= 0) {
            float hitboxBottomOffsetFromOrigin = (customHitbox_local.top + customHitbox_local.height) - (static_cast<float>(frameHeight) / 2.f);
            float targetY = defaultGroundY - hitboxBottomOffsetFromOrigin * this->currentScaleY;
            setPosition(getPosition().x, targetY);
            velocity.y = 0;
            onGround = true;
        }
        return;
    }

    if (isShooting && currentAnimationName == "shoot") {
        if (currentShootCooldown < 3) {
            isShooting = false; // Done shooting
            // Revert to idle or run based on state (velocity.x was set to 0 during shoot)
            if (onGround) { // Velocity.x should be 0 if just finished shooting on ground
                setAnimation("idle", 6, 0.1f);
            } else {
                // If shot in air, jump animation should resume or be set by falling logic
                if (!isJumping) setAnimation("jump", 10, 0.05f); // Or a "falling" animation
            }
        }
    }

    if (!onGround) {
        velocity.y += gravityForce;
    }
    sprite.move(isShooting ? 0.f : velocity.x, velocity.y);

    bool wasOnGround = onGround;
    onGround = false;
    sf::FloatRect playerHitbox = getHitboxGlobalBounds();
    sf::Vector2f currentPlayerPos = getPosition();
    float playerBottomY = playerHitbox.top + playerHitbox.height;

    if (playerBottomY >= defaultGroundY && velocity.y >= 0) {
        float hitboxBottomOffsetFromOrigin = (customHitbox_local.top + customHitbox_local.height) - (static_cast<float>(frameHeight) / 2.f);
        float targetY = defaultGroundY - hitboxBottomOffsetFromOrigin * currentScaleY;
        setPosition(currentPlayerPos.x, targetY);
        velocity.y = 0;
        isJumping = false;
        onGround = true;
        isDropping = false;
        if (!isShooting && velocity.x == 0 && currentAnimationName != "idle") {
            setAnimation("idle", 6, 0.1f);
        }
    }

    for (const auto& platform : platforms) {
        if (onGround) break;

        sf::FloatRect platformBounds = platform.getBounds();
        playerHitbox = getHitboxGlobalBounds(); // refetch position might have changed

        if (playerHitbox.intersects(platformBounds)) {
            float previousPlayerBottom = playerHitbox.top + playerHitbox.height - velocity.y; // Approx previous
            if (velocity.y >= 0 && !isDropping &&
                previousPlayerBottom <= platformBounds.top + (velocity.y > 0 ? velocity.y : 5.0f) && // More robust check for previous pos
                (playerHitbox.top + playerHitbox.height) >= platformBounds.top) {
                if ((playerHitbox.left + playerHitbox.width > platformBounds.left + 1.0f) && // Small tolerance
                    (playerHitbox.left < platformBounds.left + platformBounds.width - 1.0f)) {
                    float hitboxBottomFromOriginY = (customHitbox_local.top + customHitbox_local.height) - (static_cast<float>(frameHeight) / 2.f);
                    float targetY = platformBounds.top - hitboxBottomFromOriginY * this->currentScaleY;
                    setPosition(getPosition().x, targetY);
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

    if (onGround && !isJumping && !isShooting) {
        if (velocity.x != 0 && currentAnimationName != "run") {
            setAnimation("run", 10, 0.05f);
        } else if (velocity.x == 0 && currentAnimationName != "idle") {
            setAnimation("idle", 6, 0.1f);
        }
    }
}

bool Player::wantsToShootProjectile() const { return wantsToShootFlag; }

ProjectileSpawnInfo Player::getProjectileSpawnDetails() {
    sf::Vector2f spawnPos = getPosition();
    float horizontalOffset = (customHitbox_local.width / 2.f + 10.f) * currentScaleX;
    spawnPos.x += facingRight ? horizontalOffset : -horizontalOffset;
    float hitboxCenterY_local = customHitbox_local.top + customHitbox_local.height / 2.0f;
    float spriteOriginY_local = static_cast<float>(frameHeight) / 2.0f;
    spawnPos.y += (hitboxCenterY_local - spriteOriginY_local) * currentScaleY;
    sf::Vector2f direction = facingRight ? sf::Vector2f(1.f, 0.f) : sf::Vector2f(-1.f, 0.f);
    wantsToShootFlag = false;
    return {spawnPos, direction, projectileMoveSpeed};
}

void Player::takeDamage() {
    if (healthPoints <= 0) return;
    healthPoints--;
    notifyObservers(GameEvent::PLAYER_TOOK_DAMAGE);
    std::cout << "Player took damage. HP: " << healthPoints << std::endl;
    if (healthPoints <= 0) {
        setAnimation("death", 5, 0.25f);
        velocity = {0, 0};
    } else {
        setAnimation("hurt", 5, 0.1f);
    }
}