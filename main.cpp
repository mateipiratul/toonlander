#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <memory>

#include "class_headers/World.h"
#include "class_headers/Menu.h"
#include "class_headers/GameExceptions.h"
#include "class_headers/ConcreteEntityFactory.h"
#include "class_headers/EntityFactory.h"
#include "class_headers/SoundManager.h"
#include "class_headers/Subject.h"

enum class GameState {
    INTRO_SPLASH,
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

int main() {
    std::cout << "Game Starting...\n";
    SoundManager soundManager;
    sf::RenderWindow window;

    try {
        constexpr unsigned int windowWidth = 1600, windowHeight = 900;
        window.create(sf::VideoMode({windowWidth, windowHeight}), "ToonLander", sf::Style::Default);
        window.setFramerateLimit(90);

        GameState currentState = GameState::MENU;
        sf::Clock deltaClock;
        sf::Clock introScreenTimer; // timer for intro splash
        bool introMusicStarted = false;

        Menu gameMenu(&window);
        gameMenu.addObserver(&soundManager);
        std::unique_ptr<World> gameWorld = nullptr;
        std::unique_ptr<EntityFactory> entityFactory = std::make_unique<ConcreteEntityFactory>();

        sf::Font introFont;
        if (!introFont.loadFromFile("assets/ARCADECLASSIC.TTF")) {
            throw ResourceLoadError("Font", "assets/ARCADECLASSIC.TTF", "Failed to load intro font.");
        }
        sf::Text introTextLine1("Coq Studios", introFont, 60);
        sf::Text introTextLine2("made in SFML", introFont, 40);

        sf::FloatRect bounds1 = introTextLine1.getLocalBounds();
        introTextLine1.setOrigin(bounds1.left + bounds1.width / 2.f, bounds1.top + bounds1.height / 2.f);
        introTextLine1.setPosition(static_cast<float>(windowWidth) / 2.f, static_cast<float>(windowHeight) / 2.f - 30.f);

        sf::FloatRect bounds2 = introTextLine2.getLocalBounds();
        introTextLine2.setOrigin(bounds2.left + bounds2.width / 2.f, bounds2.top + bounds2.height / 2.f);
        introTextLine2.setPosition(static_cast<float>(windowWidth) / 2.f, static_cast<float>(windowHeight) / 2.f + 40.f);

        const float introDuration = 5.f;
        const float fadeInDuration = 1.5f;
        const float holdDuration = 2.f;
        const float fadeOutStartTime = fadeInDuration + holdDuration;
        const float fadeOutDuration = introDuration - fadeOutStartTime;

        sf::Font pauseFont;
        if (!pauseFont.loadFromFile("assets/ARCADECLASSIC.TTF")) {
            throw ResourceLoadError("Font", "assets/ARCADECLASSIC.TTF", "Failed to load pause font.");
        }
        sf::Text pauseText("PAUSED", pauseFont, 50);
        pauseText.setFillColor(sf::Color::White);
        sf::FloatRect textBounds = pauseText.getLocalBounds();
        pauseText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        pauseText.setPosition(static_cast<float>(windowWidth) / 2.f, static_cast<float>(windowHeight) / 2.f);

        sf::RectangleShape pauseOverlay;
        pauseOverlay.setSize(sf::Vector2f(static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }

                // event handling per state
                switch (currentState) {
                    case GameState::INTRO_SPLASH:
                        break;
                    case GameState::MENU:
                        gameMenu.handleInput(event);
                        break;
                    case GameState::PLAYING:
                        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                            currentState = GameState::PAUSED;
                            std::cout << "Game Paused!\n";
                        }
                        break;
                    case GameState::PAUSED:
                        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                            currentState = GameState::PLAYING;
                            deltaClock.restart();
                            std::cout << "Game Resumed\n";
                        }
                        break;
                    case GameState::GAME_OVER:
                        if (event.type == sf::Event::KeyPressed &&
                            (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Escape)) {
                            currentState = GameState::MENU;
                            gameWorld.reset();
                            if (!entityFactory) {
                                entityFactory = std::make_unique<ConcreteEntityFactory>();
                            }
                        }
                        break;
                }
            }

            float dt = 0.0f; // calculate delta time
            if (currentState != GameState::PAUSED) {
                dt = deltaClock.restart().asSeconds();
            }

            switch (currentState) { // update logic per state
                case GameState::MENU:
                    gameMenu.update(dt);
                    if (gameMenu.isStartRequested()) {
                        if (!entityFactory) {
                             entityFactory = std::make_unique<ConcreteEntityFactory>();
                        }
                        gameWorld = std::make_unique<World>(&window, std::move(entityFactory));

                        currentState = GameState::PLAYING;
                        std::cout << "Transitioning to PLAYING state\n";
                    }
                    break;
                case GameState::PLAYING:
                    if (gameWorld) {
                        gameWorld->handleInput();
                        gameWorld->update(dt);
                        if (gameWorld->isGameOver()) {
                            std::cout << "Game Over!\n";
                            currentState = GameState::GAME_OVER;
                        }
                    } else {
                        throw GameLogicError("Attempted to update null game world in PLAYING state.");
                    }
                    break;
                case GameState::PAUSED:
                    break;
                case GameState::GAME_OVER:
                    break;
            }

            window.clear();
            switch (currentState) {
                case GameState::MENU:
                    gameMenu.draw();
                    break;
                case GameState::PLAYING:
                    if (gameWorld) gameWorld->draw();
                    break;
                case GameState::PAUSED:
                    if (gameWorld) gameWorld->draw();
                    window.draw(pauseOverlay);
                    window.draw(pauseText);
                    break;
                case GameState::GAME_OVER:
                    if (gameWorld) gameWorld->draw();
                    break;
            }
            window.display();
        }
    } catch (const ResourceLoadError& e) {
        std::cerr << "\n--- RESOURCE ERROR CAUGHT ---\n" << e.what() << std::endl;
        return 1;
    } catch (const ConfigurationError& e) {
        std::cerr << "\n--- CONFIGURATION ERROR CAUGHT ---\n" << e.what() << std::endl;
        return 1;
    } catch (const GameLogicError& e) {
        std::cerr << "\n--- GAME LOGIC ERROR CAUGHT ---\n" << e.what() << std::endl;
        return 1;
    } catch (const GameError& e) {
        std::cerr << "\n--- GENERIC GAME ERROR CAUGHT ---\n" << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n--- STANDARD EXCEPTION CAUGHT ---\n" << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n--- UNKNOWN EXCEPTION CAUGHT ---" << std::endl;
        return 1;
    }

    std::cout << "Game Closing...\n";
    return 0;
}