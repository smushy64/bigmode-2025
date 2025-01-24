/**
 * @file   gui.cpp
 * @brief  GUI helper functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "gui.h"
#include "blit.h"
#include <math.h>

bool draw_pause_menu() {
}

bool draw_options_menu( GuiOptionsMenu& state ) {
    Vector2 screen = get_screen();

    Rectangle options_rect;
    options_rect.width  = screen.x - 800.0;
    options_rect.height = screen.y - 100.0;

    options_rect.width  = fmax( options_rect.width, 400.0 );
    options_rect.height = fmax( options_rect.height, 500.0 );

    options_rect.x = (screen.x / 2.0) - (options_rect.width / 2.0);
    options_rect.y = (screen.y / 2.0) - (options_rect.height / 2.0);
    if( GuiWindowBox( options_rect, "Options" ) ) {
        return true;
    }

    return false;
}
bool draw_credits_menu() {
    Vector2 screen = get_screen();

    Rectangle credits_rect;
    credits_rect.width  = screen.x - 800.0;
    credits_rect.height = screen.y - 100.0;

    credits_rect.width  = fmax( credits_rect.width, 400.0 );
    credits_rect.height = fmax( credits_rect.height, 500.0 );

    credits_rect.x = (screen.x / 2.0) - (credits_rect.width / 2.0);
    credits_rect.y = (screen.y / 2.0) - (credits_rect.height / 2.0);
    if( GuiWindowBox( credits_rect, "Credits" ) ) {
        return true;
    }

    Rectangle group_box_rect;

    float inset = 10.0;

    Rectangle left_rect = credits_rect;
    left_rect.x      += 10.0 + inset;
    left_rect.width  -= 20.0 + (inset * 2.0);

    left_rect.y      += 25.0 + (inset * 2.0);
    left_rect.height  = 24.0;

    Rectangle right_rect = left_rect;
    right_rect.x     += left_rect.width / 2.0;
    right_rect.width /= 2.0;

    group_box_rect = left_rect;
    group_box_rect.x     -= inset;
    group_box_rect.width += 10.0 + inset;

    group_box_rect.y -= inset;

    GuiLabel( left_rect,  "Alicia Amarilla" );
    GuiLabel( right_rect, "Programming and Art" );
    right_rect.y = left_rect.y += 24.0;
    group_box_rect.height += 24.0;

    GuiLabel( left_rect, "Sergio Marmarian" );
    GuiLabel( right_rect, "Art" );
    right_rect.y = left_rect.y += 24.0;
    group_box_rect.height += 24.0;

    GuiLabel( left_rect, "Jack Ma" );
    GuiLabel( right_rect, "Art" );
    right_rect.y = left_rect.y += 24.0;
    group_box_rect.height += 24.0;

    GuiLabel( left_rect, "Clayton Dryden" );
    GuiLabel( right_rect, "Music and Sound Effects" );
    group_box_rect.height += 24.0;

    GuiGroupBox( group_box_rect, "Team" );

    group_box_rect.y      += group_box_rect.height;
    group_box_rect.height  = 0.0;

    // TODO(alicia): additional credits
    return false;
}


