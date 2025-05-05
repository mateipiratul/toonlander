#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "GameExceptions.h"

class Menu {
    sf::RenderWindow* window;
    sf::Font font;
    sf::Text title;
    sf::Text startButtonText;
    sf::RectangleShape buttonBox;

    sf::Texture backgroundTexture;
    sf::Sprite bgSpr1, bgSpr2;
    const float scrollSpeed = 30.f;

    bool startRequested = false;
    bool isHovering = false;

    void centerOrigin(sf::Text& text) {
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    }

    void loadBackground() {
        if (!backgroundTexture.loadFromFile("assets/backgrounds/bg_menu.png")) {
            throw ResourceLoadError("Background", "assets/backgrounds/bg_menu.png", "Failed to load background image for menu.");
        }

        bgSpr1.setTexture(backgroundTexture);
        bgSpr2.setTexture(backgroundTexture);

        sf::Vector2u textureSize = backgroundTexture.getSize();
        sf::Vector2u windowSize = window->getSize();
        float scaleX = static_cast<float>(windowSize.x) / static_cast<float>(textureSize.x);
        float scaleY = static_cast<float>(windowSize.y) / static_cast<float>(textureSize.y);
        float bgScale = std::max(scaleX, scaleY);

        bgSpr1.setScale(bgScale, bgScale);
        bgSpr2.setScale(bgScale, bgScale);

        bgSpr1.setPosition(0.f, 0.f);
        bgSpr2.setPosition(bgSpr1.getGlobalBounds().width, 0.f);
    }

public:
    explicit Menu(sf::RenderWindow* win) : window(win) {
        if (!window) {
            throw GameLogicError("Menu requires a valid RenderWindow pointer!");
        }
        loadBackground();
        if (!font.loadFromFile("assets/ARCADECLASSIC.TTF")) {
            throw ResourceLoadError("Font", "assets/ARCADECLASSIC.TTF", "Failed to load menu font.");
        }

        title.setFont(font);
        title.setString("TOONLANDER");
        title.setCharacterSize(80);
        title.setFillColor(sf::Color::White);
        centerOrigin(title);
        title.setPosition(window->getSize().x / 2.0f, window->getSize().y * 0.25f);

        startButtonText.setFont(font);
        startButtonText.setString("START");
        startButtonText.setCharacterSize(48);
        startButtonText.setFillColor(sf::Color::Black);
        centerOrigin(startButtonText);
        startButtonText.setPosition(window->getSize().x / 2.0f, window->getSize().y * 0.6f);

        sf::FloatRect textBounds = startButtonText.getLocalBounds();
        float paddingX = 40.f;
        float paddingY = 20.f;
        buttonBox.setSize({textBounds.width + paddingX, textBounds.height * 2.0f + paddingY});
        buttonBox.setOrigin(buttonBox.getSize().x / 2.0f, buttonBox.getSize().y / 2.0f);
        buttonBox.setFillColor(sf::Color::White);
        buttonBox.setOutlineColor(sf::Color(50, 50, 50));
        buttonBox.setOutlineThickness(2.f);
        buttonBox.setPosition(startButtonText.getPosition());

    }

    void draw() const {
        window->draw(bgSpr1);
        window->draw(bgSpr2);
        window->draw(title);
        window->draw(buttonBox);
        window->draw(startButtonText);
    }

    void handleInput(const sf::Event& event) {
        sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(*window));

        isHovering = buttonBox.getGlobalBounds().contains(mousePos);

        if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                if (isHovering) {
                    startRequested = true;
                    std::cout << "Start button clicked!" << std::endl;
                }
            }
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
             startRequested = true;
             std::cout << "Start requested by Enter key!" << std::endl;
        }
    }

    void update(float dt) {
        bgSpr1.move(-scrollSpeed * dt, 0.f);
        bgSpr2.move(-scrollSpeed * dt, 0.f);

        float spriteWidth = bgSpr1.getGlobalBounds().width;
        if (bgSpr1.getPosition().x + spriteWidth <= 0)
            bgSpr1.setPosition(bgSpr2.getPosition().x + spriteWidth, 0.f);
        if (bgSpr2.getPosition().x + spriteWidth <= 0)
            bgSpr2.setPosition(bgSpr1.getPosition().x + spriteWidth, 0.f);


        if (isHovering) {
            buttonBox.setFillColor(sf::Color(200, 200, 200));
            startButtonText.setFillColor(sf::Color(50, 50, 50));
        } else {
            buttonBox.setFillColor(sf::Color::White);
            startButtonText.setFillColor(sf::Color::Black);
        }
    }

    bool isStartRequested() const {
        return startRequested;
    }

    void reset() {
        startRequested = false;
        isHovering = false;
        buttonBox.setFillColor(sf::Color::White);
        startButtonText.setFillColor(sf::Color::Black);
    }
};

#endif // MENU_H