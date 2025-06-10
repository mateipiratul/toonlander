#ifndef ENTITY_H
#define ENTITY_H

#include <SFML/Graphics.hpp>
#include <string>
#include <map>

class Entity {
protected:
    sf::Sprite sprite;
    sf::Vector2f velocity{0.f, 0.f}; // velocity of entity
    int frameWidth{0}, frameHeight{0}; // sprite measures
    int healthPoints{1}; // default hp
    sf::RenderWindow* window; // pointer to window
    std::map<std::string, sf::Texture> animationTextures;
    std::string currentAnimationName{"none"};
    sf::IntRect currentFrameRect;
    sf::Clock animationClock;
    float currentAnimationInterval{0.1f};
    int currentNumFrames{0};
    int currentFrameIndex{0};
    float currentScaleX{1.f};
    float currentScaleY{1.f};

    // load a specific animation for an entity
    bool loadAnimationTexture(const std::string& animationName, const std::string& texturePath);

public:
    // constructor takes a pointer to the render window
    explicit Entity(sf::RenderWindow* win);

    // pure virtual functions
    virtual void draw() = 0; // draws the entity to the window
    virtual void actions() = 0; // handles entity-specific actions
    virtual void takeDamage() = 0; // handles damage taken by the entity

    // virtual functions
    virtual void update(); // update the entity state
    virtual void setAnimation(const std::string& animationName, int numFrames, float interval); // set animation
    virtual void setScale(float scaleX, float scaleY); // set scale of entity sprite

    // public getters/setters
    int getHealthPoints() const;
    sf::FloatRect getVisualBounds() const; // returns global bounding box of sprite
    sf::Vector2f getPosition() const; // returns current position of sprite
    void setPosition(float x, float y); // set position of sprite using x, y coordinates.
    void setPosition(const sf::Vector2f& pos); // set position of sprite using 2d vector

    // virtual destructor
    virtual ~Entity() = default; // deleting derived objects via base pointers
};

#endif // ENTITY_H