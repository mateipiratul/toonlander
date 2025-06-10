#include "../class_headers/SoundManager.h"
#include <iostream>
#include <SFML/Audio.hpp>

SoundManager::SoundManager() {
    try {
        // load songs
        if (!introTheme.openFromFile("./assets/audio_assets/intro.wav")) {
            throw std::runtime_error("Failed to load intro song");
        }
        introTheme.setLoop(false);

        if (!menuTheme.openFromFile("./assets/audio_assets/menu_tool_7empest.wav")) {
            throw std::runtime_error("Failed to be 7empest");
        }
        menuTheme.setLoop(true);

        // load sound effect buffers
        if (!loadSoundBuffer("jump_sfx", "./assets/audio_assets/player_jump.wav"))
            { throw std::runtime_error("Error loading player_jump"); }
        if (!loadSoundBuffer("player_hurt_sfx", "./assets/audio_assets/player_hurt.wav"))
            { throw std::runtime_error("Error loading player_hurt"); }
        if (!loadSoundBuffer("button_click_sfx", "./assets/audio_assets/button_click.wav"))
            { throw std::runtime_error("Error loading button_click"); }

        // --- Link GameEvents to Loaded Sound Buffers ---
        linkEventToSound(GameEvent::PLAYER_JUMPED, "jump_sfx");
        linkEventToSound(GameEvent::PLAYER_TOOK_DAMAGE, "player_hurt_sfx");
        linkEventToSound(GameEvent::BUTTON_CLICKED, "button_click_sfx");

    } catch (const std::runtime_error& e) {
        std::cerr << "SoundManager Construction Error: " << e.what() << std::endl;
    }
}

bool SoundManager::loadSoundBuffer(const std::string& soundName, const std::string& filename) {
    if (soundBuffers.contains(soundName)) {
        std::cout << "  SoundBuffer '" << soundName << "' already loaded." << std::endl;
        return true; // already loaded
    }
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename)) {
        std::cerr << "  Failed to load sound buffer: " << filename << " for name " << soundName << std::endl;
        return false;
    }
    soundBuffers[soundName] = buffer;
    std::cout << "  Loaded SoundBuffer '" << soundName << "' from " << filename << std::endl;
    return true;
}

void SoundManager::linkEventToSound(GameEvent event, const std::string& soundName) {
    if (soundBuffers.contains(soundName)) {
        eventActionSounds[event].setBuffer(soundBuffers.at(soundName)); // Use .at for checked access to buffer
        std::cout << "  Linked GameEvent " << static_cast<int>(event) << " to sound '" << soundName << "'" << std::endl;
    } else {
        std::cerr << "SoundManager Error: Cannot link event " << static_cast<int>(event)
                  << " to sound '" << soundName << "'. Buffer not found." << std::endl;
    }
}

void SoundManager::playSoundForEvent(GameEvent event) {
    if (eventActionSounds.contains(event)) {
        eventActionSounds.at(event).play();
    } else {
        std::cerr << "SoundManager Error: No sound linked for event " << static_cast<int>(event) << std::endl;
    }
}

void SoundManager::onNotify(GameEvent event) {
    switch (event) {
        case GameEvent::PLAYER_JUMPED:
        case GameEvent::PLAYER_TOOK_DAMAGE:
        case GameEvent::BUTTON_CLICKED:
            playSoundForEvent(event); // play the sound linked to this event
            break;

        case GameEvent::GAME_STARTED:
            if (isMenuThemePlaying()) { stopMenuTheme(); }
            playIntroTheme();
            break;
        case GameEvent::MENU_ENTERED: // this event signals the menu is now active
            if (isIntroThemePlaying()) { stopIntroTheme(); }
            playMenuTheme();
            break;
        case GameEvent::GAMEPLAY_STARTED:
            stopMenuTheme();
        default:
            std::cout << "SoundManager: No specific sound action for event " << static_cast<int>(event) << std::endl;
            break;
    }
}

// player functions
void SoundManager::playIntroTheme() {
    if (introTheme.getStatus() != sf::Music::Playing) {
        introTheme.play();
    }
}

void SoundManager::stopIntroTheme() {
    if (introTheme.getStatus() == sf::Music::Playing) {
        introTheme.stop();
    }
}

bool SoundManager::isIntroThemePlaying() const {
    return introTheme.getStatus() == sf::Music::Playing;
}

void SoundManager::playMenuTheme() {
    if (menuTheme.getStatus() != sf::Music::Playing) {
        menuTheme.play();
    }
}

void SoundManager::stopMenuTheme() {
    if (menuTheme.getStatus() == sf::Music::Playing) {
        menuTheme.stop();
    }
}

bool SoundManager::isMenuThemePlaying() const {
    return menuTheme.getStatus() == sf::Music::Playing;
}

void SoundManager::setGlobalSoundVolume(float volume) {
    for (auto& pair : eventActionSounds) {
        pair.second.setVolume(volume);
    }
}

void SoundManager::setGlobalMusicVolume(float volume) {
    introTheme.setVolume(volume);
    menuTheme.setVolume(volume);
}