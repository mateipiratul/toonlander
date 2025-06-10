#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include <SFML/Audio.hpp>
#include <map>
#include <string>
#include "Observer.h"
#include "GameEvents.h"

class SoundManager : public Observer {
    std::map<std::string, sf::SoundBuffer> soundBuffers; // stores actual audio data
    std::map<GameEvent, sf::Sound> eventActionSounds; // maps game events to playable sounds
    sf::Music introTheme; // specific intro
    sf::Music menuTheme; // specific menu loop

    // helper functions
    bool loadSoundBuffer(const std::string& name, const std::string& filename);
    void linkEventToSound(GameEvent event, const std::string& soundName);
    void playSoundForEvent(GameEvent event);

public:
    SoundManager(); // loading resources
    ~SoundManager() override = default;

    // core observer pattern implementation
    void onNotify(GameEvent event) override;

    // players
    void playIntroTheme();
    void stopIntroTheme();
    bool isIntroThemePlaying() const;

    void playMenuTheme();
    void stopMenuTheme();
    bool isMenuThemePlaying() const;

    // general volume controls
    void setGlobalSoundVolume(float volume);
    void setGlobalMusicVolume(float volume);
};

#endif //SOUNDMANAGER_H
