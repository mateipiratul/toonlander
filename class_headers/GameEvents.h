#ifndef GAMEEVENTS_H
#define GAMEEVENTS_H

enum class GameEvent {
  PLAYER_JUMPED,         // jump sound
  PLAYER_TOOK_DAMAGE,    // hurt sound
  BUTTON_CLICKED,        // button click
  GAME_STARTED,          // intro sound
  MENU_ENTERED,          // menu music
  GAMEPLAY_STARTED       // stop menu music
};

#endif //GAMEEVENTS_H