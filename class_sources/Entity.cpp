#include "../class_headers/Entity.h"
#include "../class_headers/GameExceptions.h"
#include <iostream>

Entity::Entity(sf::RenderWindow* win) : window(win) {}

bool Entity::loadAnimationTexture(const std::string& animationName, const std::string& texturePath) {
    if (animationTextures.contains(animationName)) { return true; }

    sf::Texture tex;
    if (!tex.loadFromFile(texturePath)) {
        throw ResourceLoadError("Texture (Animation)", texturePath, "Failed in Entity::loadAnimationTexture for animation: " + animationName);
    }
    tex.setSmooth(false); // disable smoothing
    animationTextures[animationName] = tex; // store the loaded texture in the map
    return true;
}

void Entity::setAnimation(const std::string& animationName, int numFrames, float interval) {
    if (!animationTextures.count(animationName)) {
        throw ResourceLoadError("Animation Texture", animationName, "Attempted to set unloaded animation. Texture not found in map.");
    }

    if (currentAnimationName != animationName || sprite.getTexture() == nullptr || currentNumFrames != numFrames || currentAnimationInterval != interval) {
        currentAnimationName = animationName;
        currentNumFrames = numFrames;
        currentAnimationInterval = interval;
        currentFrameIndex = 0;

        sprite.setTexture(animationTextures.at(animationName), true);

        if (frameWidth <= 0 || frameHeight <= 0) {
            throw InvalidStateError("Frame dimensions for Entity::setAnimation");
        }
        currentFrameRect = sf::IntRect(0, 0, frameWidth, frameHeight);
        sprite.setTextureRect(currentFrameRect);
        sprite.setOrigin(static_cast<float>(frameWidth) / 2.0f, static_cast<float>(frameHeight) / 2.0f);

        float existingSignX = (sprite.getScale().x < 0.0f) ? -1.0f : 1.0f;
        // Ensure currentScaleX/Y are the base absolute scales from Entity::setScale()
        sprite.setScale(existingSignX * this->currentScaleX, this->currentScaleY);

        animationClock.restart();
    }
}

// virtual functions
void Entity::update() {
    if (currentAnimationInterval <= 0 || currentNumFrames <= 0 || currentAnimationName.empty() || !animationTextures.count(currentAnimationName)) {
        return; // Not a valid animation to update
    }

    if (animationClock.getElapsedTime().asSeconds() >= currentAnimationInterval) {
        if (currentFrameIndex == currentNumFrames - 1) { // Currently on the last frame, and its time has just elapsed
            currentFrameIndex = 0; // Loop for next cycle
        } else {
            currentFrameIndex++;   // Go to the next frame
        }

        if (frameWidth > 0) {
            currentFrameRect.left = currentFrameIndex * frameWidth;
            sprite.setTextureRect(currentFrameRect);
        }
        animationClock.restart(); // Restart clock for the NEWLY set frame
    }
}


void Entity::setScale(float scaleX, float scaleY) {
    currentScaleX = std::abs(scaleX);
    currentScaleY = std::abs(scaleY);
    sprite.setScale(scaleX, scaleY);
}

// getters/setters
sf::FloatRect Entity::getVisualBounds() const { return sprite.getGlobalBounds(); }

sf::Vector2f Entity::getPosition() const { return sprite.getPosition(); }

void Entity::setPosition(float x, float y) { sprite.setPosition(x, y);}

void Entity::setPosition(const sf::Vector2f& pos) { sprite.setPosition(pos); }

int Entity::getHealthPoints() const { return healthPoints; }