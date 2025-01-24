#if !defined(STATE_H)
#define STATE_H
/**
 * @file   state.h
 * @brief  Game state.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "modes.h"

struct GlobalState {
    float timer;

    Mode mode;
    union {
        struct {
            Texture bigmode_logo;
        } intro;
        struct {

        } main_menu;
        struct {

        } game;
    };

    void load_mode( Mode mode );
    void unload_mode( Mode mode );

    inline
    void set_mode( Mode mode ) {
        unload_mode( mode );
        load_mode( mode );
    }
};

#endif /* header guard */
