#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include "Subject.h"

class Menu : public Subject {
    sf::RenderWindow* window;
    sf::Font font;
    sf::Text title;
    sf::Text startButtonText;
    sf::RectangleShape buttonBox;

    sf::Texture backgroundTexture;
    sf::Sprite bgSpr1, bgSpr2;
    const float scrollSpeed{30.f};

    bool startRequested{false};
    bool isHovering{false};

    static void centerOrigin(sf::Text& text);
    void loadBackground();

public:
    explicit Menu(sf::RenderWindow* win);

    void draw() const;
    void handleInput(const sf::Event& event);
    void update(float dt);
    bool isStartRequested() const;
};

#endif // MENU_H