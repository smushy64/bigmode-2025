/**
 * @file   main_menu.cpp
 * @brief  Main menu mode.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "modes.h"
#include "state.h"
#include "blit.h"

void mode_main_menu_load( GlobalState* state ) {

}
void mode_main_menu_update( GlobalState* state, float dt ) {
    BeginDrawing();
    ClearBackground( BLACK );

    Vector2 screen = get_screen();

    if( !(
        state->transient.main_menu.is_credits_open ||
        state->transient.main_menu.is_options_open
    ) ) {

        Vector2 text_measure = MeasureTextEx(
            state->persistent.font, "BIGMODE Game Jam 2025", 24.0, 1.0 );
        DrawTextPro(
            state->persistent.font, "BIGMODE Game Jam 2025",
            {screen.x / 2.0f, (screen.y / 2.0f) - 24.0f},
            text_measure / 2.0, 0.0, 24.0, 1.0, WHITE );

        Rectangle button_rect = { screen.x / 2.0f, screen.y / 2.0f, 95.0, 35.0 };
        button_rect.x -= button_rect.width / 2.0;

        float spacing = button_rect.height + 6.0;

        if( GuiButton( button_rect, "Start Game" ) ) {
            mode_set( state, Mode::GAME );
            EndDrawing();
            return;
        }

        button_rect.y += spacing;

        if( GuiButton( button_rect, "Options" ) ) {
            state->transient.main_menu.is_options_open =
                !state->transient.main_menu.is_options_open;
        }

        button_rect.y += spacing;

        if( GuiButton( button_rect, "Credits" ) ) {
            state->transient.main_menu.is_credits_open =
                !state->transient.main_menu.is_credits_open;
        }

#if !defined(PLATFORM_WEB)
        button_rect.y += spacing;

        if( GuiButton( button_rect, "Quit Game" ) ) {
            game_exit();
        }
#endif

    } else {
        if( state->transient.main_menu.is_options_open ) {
            if( draw_options_menu( state->transient.main_menu.options_state ) ) {
                state->transient.main_menu.is_options_open = false;
            }
        }

        if( state->transient.main_menu.is_credits_open ) {
            if( draw_credits_menu() ) {
                state->transient.main_menu.is_credits_open = false;
            }
        }
    }

    EndDrawing();
}
void mode_main_menu_unload( GlobalState* state ) {

}

