#include "../class_headers/Entity.h"
#include "../class_headers/GameExceptions.h"
#include <iostream>

// constructor sets the render window pointer
Entity::Entity(sf::RenderWindow* win) : window(win) {}

bool Entity::loadAnimationTexture(const std::string& animationName, const std::string& texturePath) {
    if (animationTextures.contains(animationName)) { return true; } // already loaded

    sf::Texture tex;
    if (!tex.loadFromFile(texturePath)) {
        // throw error if texture fails to load
        throw ResourceLoadError("Texture (Animation)", texturePath, "Failed in Entity::loadAnimationTexture for animation: " + animationName);
    }
    tex.setSmooth(false); // disable smoothing for pixel art
    animationTextures[animationName] = tex; // store texture in map
    return true;
}

void Entity::setAnimation(const std::string& animationName, int numFrames, float interval) {
    if (!animationTextures.count(animationName)) {
        // throw error if animation texture not found
        throw ResourceLoadError("Animation Texture", animationName, "Attempted to set unloaded animation. Texture not found in map.");
    }

    // only reset animation if parameters changed
    if (currentAnimationName != animationName || sprite.getTexture() == nullptr || currentNumFrames != numFrames || currentAnimationInterval != interval) {
        currentAnimationName = animationName;
        currentNumFrames = numFrames;
        currentAnimationInterval = interval;
        currentFrameIndex = 0;

        sprite.setTexture(animationTextures.at(animationName), true);

        if (frameWidth <= 0 || frameHeight <= 0) {
            // throw error if frame size is invalid
            throw InvalidStateError("Frame dimensions for Entity::setAnimation");
        }

        currentFrameRect = sf::IntRect(0, 0, frameWidth, frameHeight);
        sprite.setTextureRect(currentFrameRect);
        sprite.setOrigin(static_cast<float>(frameWidth) / 2.0f, static_cast<float>(frameHeight) / 2.0f);

        float existingSignX = (sprite.getScale().x < 0.0f) ? -1.0f : 1.0f;
        // apply correct scale with direction
        sprite.setScale(existingSignX * this->currentScaleX, this->currentScaleY);

        animationClock.restart(); // restart animation timing
    }
}

// update animation frame based on time
void Entity::update() {
    if (currentAnimationInterval <= 0 || currentNumFrames <= 0 || currentAnimationName.empty() || !animationTextures.count(currentAnimationName)) {
        return; // invalid animation data
    }

    if (animationClock.getElapsedTime().asSeconds() >= currentAnimationInterval) {
        // go to next frame or loop
        if (currentFrameIndex == currentNumFrames - 1) {
            currentFrameIndex = 0; // loop animation
        } else {
            currentFrameIndex++; // go to next frame
        }

        if (frameWidth > 0) {
            currentFrameRect.left = currentFrameIndex * frameWidth;
            sprite.setTextureRect(currentFrameRect);
        }

        animationClock.restart(); // reset time for next frame
    }
}

// set sprite scale and store absolute scale
void Entity::setScale(float scaleX, float scaleY) {
    currentScaleX = std::abs(scaleX);
    currentScaleY = std::abs(scaleY);
    sprite.setScale(scaleX, scaleY);
}

// return sprite global bounds
sf::FloatRect Entity::getVisualBounds() const { return sprite.getGlobalBounds(); }

// return sprite position
sf::Vector2f Entity::getPosition() const { return sprite.getPosition(); }

// set position by x and y
void Entity::setPosition(float x, float y) { sprite.setPosition(x, y);}

// set position by vector
void Entity::setPosition(const sf::Vector2f& pos) { sprite.setPosition(pos); }

// return current health points
int Entity::getHealthPoints() const { return healthPoints; }