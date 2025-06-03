#include "../class_headers/GameExceptions.h"
#include "../class_headers/Menu.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

void Menu::centerOrigin(sf::Text& text) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
}

void Menu::loadBackground() {
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

Menu::Menu(sf::RenderWindow* win) : window(win) {
    if (!window) { throw GameLogicError("Menu requires a valid RenderWindow pointer!"); }
    try {
        loadBackground();

        if (!font.loadFromFile("assets/ARCADECLASSIC.TTF")) {
            throw ResourceLoadError("Font", "assets/ARCADECLASSIC.TTF", "Failed to load menu font.");
        }

        title.setFont(font);
        title.setString("TOONLANDER");
        title.setCharacterSize(80);
        title.setFillColor(sf::Color::White);
        centerOrigin(title);
        title.setPosition(static_cast<float>(window->getSize().x) / 2.f, static_cast<float>(window->getSize().y) * 0.25f);

        startButtonText.setFont(font);
        startButtonText.setString("START");
        startButtonText.setCharacterSize(48);
        startButtonText.setFillColor(sf::Color::Black);
        centerOrigin(startButtonText);
        startButtonText.setPosition(static_cast<float>(window->getSize().x) / 2.0f, static_cast<float>(window->getSize().y) * 0.6f);

        sf::FloatRect textBounds = startButtonText.getLocalBounds(); // use local bounds as pre-transform
        float paddingX = 40.f;
        float paddingY = 20.f;
        // size box based on text dimensions before centering origin
        buttonBox.setSize({textBounds.width + paddingX, textBounds.height * 2.0f + paddingY});
        buttonBox.setOrigin(buttonBox.getSize().x / 2.0f, buttonBox.getSize().y / 2.0f);
        buttonBox.setFillColor(sf::Color::White);
        buttonBox.setOutlineColor(sf::Color(50, 50, 50));
        buttonBox.setOutlineThickness(2.f);
        buttonBox.setPosition(startButtonText.getPosition()); // align with the already centered text
    } catch (const ResourceLoadError& e) {
        std::cerr << "Error during Menu construction (ResourceLoadError): " << e.what() << std::endl;
        throw; // re-throwing the original exception
    } catch (const GameLogicError& e) {
        std::cerr << "Error during Menu construction (GameLogicError): " << e.what() << std::endl;
        throw;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception during Menu construction: " << e.what() << std::endl;
        throw GameError("A standard error occurred during menu initialization."); // wrap in a common game error
    } catch (...) {
        std::cerr << "Unknown exception during Menu construction." << std::endl;
        throw GameError("An unknown error occurred during menu initialization."); // wrap in a common game error
    }
}

void Menu::draw() const { // draw all elements on screen
    window->draw(bgSpr1);
    window->draw(bgSpr2);
    window->draw(title);
    window->draw(buttonBox);
    window->draw(startButtonText);
}

void Menu::handleInput(const sf::Event& event) {
    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(*window));

    isHovering = buttonBox.getGlobalBounds().contains(mousePos);

    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            if (isHovering) {
                startRequested = true;
                std::cout << "Start button clicked!" << std::endl;
                notifyObservers(GameEvent::BUTTON_CLICKED);
            }
        }
    }
    if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
        startRequested = true;
        std::cout << "Start requested by Enter key!" << std::endl;
    }
}

void Menu::update(float dt) {
    bgSpr1.move(-scrollSpeed * dt, 0.f);
    bgSpr2.move(-scrollSpeed * dt, 0.f);

    float spriteWidth = bgSpr1.getGlobalBounds().width;
    if (bgSpr1.getPosition().x + spriteWidth <= 0) {
        // when bgSpr1 moves completely off-screen to the left reposition it to the right of bgSpr2
        bgSpr1.setPosition(bgSpr2.getPosition().x + spriteWidth - (scrollSpeed * dt), 0.f); // avoid gaps on scroll
    }
    if (bgSpr2.getPosition().x + spriteWidth <= 0) {
        // when bgSpr2 moves completely off-screen to the left reposition it to the right of bgSpr1
        bgSpr2.setPosition(bgSpr1.getPosition().x + spriteWidth - (scrollSpeed * dt), 0.f);
    }

    if (isHovering) {
        buttonBox.setFillColor(sf::Color(200, 200, 200)); // lighter gray for hover
        startButtonText.setFillColor(sf::Color(50, 50, 50)); // darker text on hover
    } else {
        buttonBox.setFillColor(sf::Color::White);
        startButtonText.setFillColor(sf::Color::Black);
    }
}

bool Menu::isStartRequested() const { return startRequested; }