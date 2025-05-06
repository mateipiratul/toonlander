#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdexcept>
#include <chrono>

#include "class_headers/World.h"
#include "class_headers/Menu.h"
#include "class_headers/GameExceptions.h"

enum class GameState {
    MENU,
    PLAYING,
    PAUSED
};

int main() {
    std::cout << "Game Starting...\n";

    sf::RenderWindow window;
    try {
        constexpr unsigned int windowWidth = 1600, windowHeight = 900;
        window.create(sf::VideoMode({windowWidth, windowHeight}), "ToonLander", sf::Style::Default);
        window.setFramerateLimit(90);
        GameState currentState = GameState::MENU;
        sf::Clock deltaClock;
        Menu gameMenu(&window);
        std::unique_ptr<World> gameWorld = nullptr;

        sf::Font pauseFont;
        if (!pauseFont.loadFromFile("assets/ARCADECLASSIC.TTF")) {
            throw ResourceLoadError("Font", "assets/ARCADECLASSIC.TTF", "Failed to load pause font.");
        }

        sf::Text pauseText("PAUSED", pauseFont, 50);
        pauseText.setFillColor(sf::Color::White);
        pauseText.setOrigin(pauseText.getLocalBounds().width / 2.f, pauseText.getLocalBounds().height / 2.f);
        pauseText.setPosition(windowWidth / 2.f, windowHeight / 2.f);
        sf::RectangleShape pauseOverlay;
        pauseOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
        pauseOverlay.setFillColor(sf::Color(0, 0, 0, 150));

        while (window.isOpen()) {
            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) { window.close(); }

                switch (currentState) {
                    case GameState::MENU:
                        gameMenu.handleInput(event);
                        break;
                    case GameState::PLAYING:
                        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                            currentState = GameState::PAUSED;
                            std::cout << "Game Paused!\n" << std::endl;
                        }
                        break;
                    case GameState::PAUSED:
                        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P) {
                            currentState = GameState::PLAYING;
                            deltaClock.restart();
                            std::cout << "Game Resumed\n";
                        }
                        break;
                    // case GameState::GAME_OVER:
                    //     break;
                }
            }

            float dt = deltaClock.restart().asSeconds();

            switch (currentState) {
                case GameState::MENU:
                    gameMenu.update(dt);
                    if (gameMenu.isStartRequested()) {
                        gameWorld = std::make_unique<World>(&window);
                        currentState = GameState::PLAYING;
                        std::cout << "Transitioning to PLAYING state\n";
                    }
                    break;
                case GameState::PLAYING:
                    if (gameWorld) {
                        gameWorld->update(dt);
                    } else {
                        throw GameLogicError("Attempted to update null game world in PLAYING state.");
                    }
                    break;
                case GameState::PAUSED:
                    break;
                // case GameState::GAME_OVER:
                //     break;
            }

            window.clear();
            switch (currentState) {
                case GameState::MENU:
                    gameMenu.draw();
                    break;
                case GameState::PLAYING:
                    if (gameWorld) { gameWorld->draw(); }
                    break;
                case GameState::PAUSED:
                    if (gameWorld) { gameWorld->draw(); }
                    window.draw(pauseOverlay);
                    window.draw(pauseText);
                    break;
                // case GameState::GAME_OVER:
                //     break;
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