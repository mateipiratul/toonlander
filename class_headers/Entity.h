#ifndef ENTITY_H
#define ENTITY_H

#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <iostream>
#include <cmath>
#include "GameExceptions.h"

class Entity {
protected:
    sf::Sprite sprite;
    sf::Vector2f velocity;
    int frameWidth, frameHeight;
    int healthPoints;

    sf::RenderWindow* window;

    std::map<std::string, sf::Texture> animationTextures;
    std::string currentAnimationName;
    sf::IntRect currentFrameRect;
    sf::Clock animationClock;
    float currentAnimationInterval;
    int currentNumFrames;
    int currentFrameIndex;
    float currentScaleX = 1.f;
    float currentScaleY = 1.f;

    void loadAnimationTexture(const std::string& animationName, const std::string& texturePath) {
        if (animationTextures.count(animationName)) return;

        sf::Texture tex;
        if (!tex.loadFromFile(texturePath)) {
            throw ResourceLoadError("Texture (Animation)", texturePath, "Failed in Entity::loadAnimationTexture for animation: " + animationName);
        }

        tex.setSmooth(false);
        animationTextures[animationName] = tex;
        std::cout << "Loaded texture for animation: " << animationName << std::endl;
    }

public:
    explicit Entity(sf::RenderWindow* win) :
        velocity(0.0f, 0.0f),
        frameWidth(0),
        frameHeight(0),
        healthPoints(0),
        window(win),
        currentAnimationName("none"),
        currentAnimationInterval(0.1f),
        currentNumFrames(1),
        currentFrameIndex(0)
        {}

    virtual void draw() = 0;
    virtual void actions() = 0;
    virtual void takeDamage() = 0;

    virtual void update() {
        if (currentAnimationInterval > 0 && currentNumFrames > 1 && !currentAnimationName.empty() && animationTextures.count(currentAnimationName)) {
            if (animationClock.getElapsedTime().asSeconds() >= currentAnimationInterval) {
                currentFrameIndex = (currentFrameIndex + 1) % currentNumFrames;

                if (frameWidth > 0) {
                    currentFrameRect.left = currentFrameIndex * frameWidth;
                    sprite.setTextureRect(currentFrameRect);
                }
                animationClock.restart();
            }
        }
    }

    virtual void setAnimation(const std::string& animationName, int numFrames, float interval) {
        if (animationTextures.find(animationName) == animationTextures.end()) {
            throw ResourceLoadError("Animation Texture", "N/A (Not Found in Map)", "Attempted to set unloaded animation: " + animationName);
        }

        if (currentAnimationName != animationName || sprite.getTexture() == nullptr) {
            currentAnimationName = animationName;
            currentNumFrames = numFrames;
            currentAnimationInterval = interval;
            currentFrameIndex = 0;

            sprite.setTexture(animationTextures[animationName], true);

            if (frameWidth > 0 && frameHeight > 0) {
                currentFrameRect = sf::IntRect(0, 0, frameWidth, frameHeight);
                sprite.setTextureRect(currentFrameRect);
                sprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);
            } else {
                throw InvalidStateError("Frame dimensions (width/height) were zero or negative when calling setAnimation for: " + animationName + ". Ensure frameWidth/frameHeight are set in derived class constructor before calling setAnimation.");
            }

            float signX = (sprite.getScale().x > 0.f) ? 1.f : -1.f;
            sprite.setScale(signX * currentScaleX, currentScaleY);
            animationClock.restart();
        }
    }

    virtual void setScale(float scaleX, float scaleY) {
        currentScaleX = std::abs(scaleX);
        currentScaleY = std::abs(scaleY);
        sprite.setScale(scaleX, scaleY);
    }

    sf::FloatRect getVisualBounds() const {
        return sprite.getGlobalBounds();
    }

    // const sf::Sprite& getSprite() const {
    //     return sprite;
    // }

    sf::Vector2f getPosition() const {
        return sprite.getPosition();
    }

    void setPosition(float x, float y) {
        sprite.setPosition(x, y);
    }

    void setPosition(const sf::Vector2f& pos) {
        sprite.setPosition(pos);
    }

    virtual ~Entity() = default;
};

#endif // ENTITY_H