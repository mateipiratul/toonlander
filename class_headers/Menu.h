#ifndef MENU_H
#define MENU_H

#include <SFML/Graphics.hpp>
#include "Subject.h"

class Menu : public Subject {
    sf::RenderWindow* window;              // pointer to the main window
    sf::Font font;                         // font used for text
    sf::Text title;                        // game title text
    sf::Text startButtonText;             // start button label
    sf::RectangleShape buttonBox;         // shape for the start button

    sf::Texture backgroundTexture;        // background image texture
    sf::Sprite bgSpr1, bgSpr2;            // two sprites for scrolling
    const float scrollSpeed{30.f};        // speed of background scroll

    bool startRequested{false};           // true if start was pressed
    bool isHovering{false};               // true if mouse on button

    static void centerOrigin(sf::Text& text); // center text origin
    void loadBackground();                   // load background assets

public:
    explicit Menu(sf::RenderWindow* win);   // constructor with window

    void draw() const;                      // draw menu to window
    void handleInput(const sf::Event& event); // handle user input
    void update(float dt);                  // update menu state
    bool isStartRequested() const;          // check start flag
};

#endif // MENU_H