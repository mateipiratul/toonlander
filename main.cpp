#include <iostream>
#include <vector>
#include <chrono>

#include <SFML/Graphics.hpp>

#include <Helper.h>

class Projectile {
    sf::RectangleShape shape; // will be replaced with an actual sprite
    float speed;
    int direction; // either 1 for right of -1 for left

public:
    Projectile(float x, float y, int dir) : speed(13.0f), direction(dir) {
        shape.setSize({17.0f, 10.0f}); // size of bullet
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y); // initial position of the bullet
    }

    void update() { shape.move(speed * direction, 0); } // linear movement

    void draw(sf::RenderWindow& win) const { win.draw(shape); }

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }

    bool isOffScreen(int screenWidth) const { return shape.getPosition().x > screenWidth; }
};

class Platform {
    sf::RectangleShape shape;

public:
    Platform(float x, float y, float width, float height) {
        shape.setSize({width, height});
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x, y);
    }

    void draw(sf::RenderWindow& win) const { win.draw(shape); }

    sf::FloatRect getBounds() const { return shape.getGlobalBounds(); }
};

class Player {
    sf::RectangleShape shape;
    const float speed = 4.0f; // movement speed
    sf::Vector2f velocity;
    bool isJumping; // jumping state
    bool canJump; // jump only once per key press
    bool isDropping; // platform drop-down state
    bool facingRight;
    const float gravity = 0.5f;
    const float jumpForce = -15.0f;
    const float groundY = 850.0f;
    sf::RenderWindow* window;

    std::vector<Projectile> bullets;
    int shootCooldown;

public:
    explicit Player(sf::RenderWindow* win) :
    velocity(0.0f, 0.0f), isJumping(false),
    canJump(true), isDropping(false), facingRight(true),
    window(win), shootCooldown(10) {
        shape.setSize({40.0f, 50.0f});
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(400, groundY);
    }

    void handleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && shape.getPosition().x > 0) {
            velocity.x = -speed;
            facingRight = false;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && shape.getPosition().x + shape.getSize().x < window->getSize().x) {
            velocity.x = speed;
            facingRight = true;
        }
        else {
            velocity.x = 0;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && !isJumping && canJump && !sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            isJumping = true;
            velocity.y = jumpForce;
            canJump = false;
        }

        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            canJump = true;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && !isJumping) {
            isDropping = true;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) && shootCooldown <= 0) {
            int bulletDirection = facingRight ? 1 : -1;
            float bulletX = facingRight ? shape.getPosition().x + shape.getSize().x + 5 : shape.getPosition().x - 20;
            bullets.emplace_back(bulletX, shape.getPosition().y + shape.getSize().y / 2 - 5, bulletDirection);
            shootCooldown = 10;
        }
    }

    void updatePhysics(const std::vector<Platform>& platforms) {
        velocity.y += gravity;
        shape.move(velocity);

        if (shape.getPosition().y >= groundY) {
            isJumping = false;
            isDropping = false;
            shape.setPosition(shape.getPosition().x, groundY);
            velocity.y = 0.0f;
        }

        if (shootCooldown > 0) {
            shootCooldown--;
        }

        for (auto bull = bullets.begin(); bull != bullets.end();) {
            bull->update();
            if (bull->isOffScreen(window->getSize().x)) {
                bull = bullets.erase(bull);
            }
            else {
                ++bull;
            }
        }

        for (const auto& platform : platforms) {
            sf::FloatRect platformBounds = platform.getBounds();
            sf::FloatRect playerBounds = shape.getGlobalBounds();

            if (playerBounds.intersects(platformBounds) && velocity.y > 0) {
                if (!isDropping) {
                    isJumping = false;
                    shape.setPosition(shape.getPosition().x, platformBounds.top - playerBounds.height);
                    velocity.y = 0.0f;
                }
            }

            if (shape.getPosition().y > platformBounds.top + platformBounds.height) { isDropping = false; }
        }
    }

    void draw(sf::RenderWindow& win) const {
        win.draw(shape);
        for (const auto& bullet : bullets) { bullet.draw(win); }
    }
};

int main() {
    std::cout << "mesaj initial\n";
    Helper helper;
    helper.help();

    sf::RenderWindow window;
    window.create(sf::VideoMode({1600, 900}), "ToonLander", sf::Style::Default);
    /// window.setVerticalSyncEnabled(true); /// varianta alternativa
    window.setFramerateLimit(120);
    Player player(&window);
    bool isPaused = false; // initial, unpaused state

    std::vector<Platform> platforms = {
        {600, 700, 300, 20},
        {700, 500, 400, 20},
        {300, 350, 250, 20}
    };

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::P)
                isPaused = !isPaused;
        }

        if (!isPaused) {
            player.handleInput();
            player.updatePhysics(platforms); // apply gravity
        }

        window.clear();
        for (const auto& platform : platforms) {
            platform.draw(window);
        }
        player.draw(window);
        window.display();
    }

    return 0;
}