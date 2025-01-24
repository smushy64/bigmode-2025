#if !defined(MODES_H)
#define MODES_H
/**
 * @file   modes.h
 * @brief  Game modes.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/

struct GlobalState;
enum class Mode {
    INTRO,
    MAIN_MENU,
    GAME,
};

void mode_intro( GlobalState* state, float dt );
void mode_main_menu( GlobalState* state, float dt );
void mode_game( GlobalState* state, float dt );

inline
const char* to_string( Mode mode ) {
    switch( mode ) {
        case Mode::INTRO:     return "Intro";
        case Mode::MAIN_MENU: return "Main Menu";
        case Mode::GAME:      return "Game";
    }
}

#endif /* header guard */
