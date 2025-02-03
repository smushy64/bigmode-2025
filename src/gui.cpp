/**
 * @file   gui.cpp
 * @brief  GUI helper functions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "gui.h"
#include "blit.h"
#include "globals.h"
#include <math.h>

extern void game_exit();

bool draw_pause_menu( GuiPauseMenu& state ) {
    Vector2 screen = get_screen();

    if( state.is_options_open ) {
        if( draw_options_menu() ) {
            state.is_options_open = false;
        }
        return true;
    }

    Rectangle r_menu;
    r_menu.width  = screen.x - (screen.x / 1.3);
    r_menu.height = screen.y - (screen.y / 1.4);
    
    r_menu.x = (screen.x / 2.0) - (r_menu.width  / 2.0);
    r_menu.y = (screen.y / 2.0) - (r_menu.height / 2.0);

#if !defined(PLATFORM_WEB)
    r_menu.height += 35.0;
#endif

    if( GuiWindowBox( r_menu, "Paused" ) ) {
        return false;
    }

    const static float gutter = 10.0;
    r_menu.x      += gutter;
    r_menu.width  -= gutter * 2.0;
    r_menu.y      += 35.0;
    r_menu.height -= 35.0;
    const static float button_spacing = 10.0;
    const static float button_height  = 30.0;
    Rectangle r_button = r_menu;
    r_button.height = button_height;

    if( GuiButton( r_button, "Resume" ) ) {
        return false;
    }
    r_button.y += button_spacing + button_height;
    if( GuiButton( r_button, "Reset Level" ) ) {
        state.reset_level = true;
        return true;
    }
    r_button.y += button_spacing + button_height;
    if( GuiButton( r_button, "Options" ) ) {
        state.is_options_open = true;
        return true;
    }
    r_button.y += button_spacing + button_height;
    if( GuiButton( r_button, "Quit to Menu" ) ) {
        state.quit_to_menu = true;
    }
    r_button.y += button_spacing + button_height;
#if !defined(PLATFORM_WEB)
    if( GuiButton( r_button, "Quit to Desktop" ) ) {
        game_exit();
    }
#endif

    return true;
}

bool draw_options_menu() {
    Vector2 screen = get_screen();

    Rectangle r_menu;
    r_menu.width  = screen.x - (screen.x / 1.4);
    r_menu.height = screen.y - (screen.y / 2.8);
    
    r_menu.x = (screen.x / 2.0) - (r_menu.width  / 2.0);
    r_menu.y = (screen.y / 2.0) - (r_menu.height / 2.0);

    if( GuiWindowBox( r_menu, "Options" ) ) {
        return true;
    }

    const static float gutter = 10.0;
    r_menu.x      += gutter;
    r_menu.width  -= gutter * 2.0;
    r_menu.y      += 35.0;
    r_menu.height -= 35.0;

    const static float button_spacing = 10.0;
    const static float button_height  = 30.0;
    Rectangle r_button = r_menu;
    r_button.height = button_height;

    Rectangle r_slider = r_button;
    float slider_move  = MeasureTextEx( GameFont(), "XXXXXXXXXXXX", 20.0, 1.0 ).x;
    r_slider.x     += slider_move;
    r_slider.width -= slider_move;

    if( GuiButton( r_button, "Return" ) ) {
        return true;
    }
    r_slider.y = r_button.y += button_spacing + button_height;

    if( GuiButton( r_button, TextFormat( "FXAA: %s", OptionFXAA() ? "ON" : "OFF" )) ) {
        OptionFXAA( !OptionFXAA() );
    }
    r_slider.y = r_button.y += button_spacing + button_height;

    Vector2 sensitivity = OptionCameraSensitivity();
    if( GuiSlider(
        r_slider, TextFormat( "Sensitivity X %.2f ", sensitivity.x ),
        "", &sensitivity.x, 0.0, 4.0
    ) ) {
        OptionCameraSensitivity( sensitivity );
    }
    r_button.y = r_slider.y += button_spacing + button_height;

    sensitivity = OptionCameraSensitivity();
    if( GuiSlider(
        r_slider, TextFormat( "Sensitivity Y %.2f ", sensitivity.y ),
        "", &sensitivity.y, 0.0, 4.0 
    ) ) {
        OptionCameraSensitivity( sensitivity );
    }
    r_button.y = r_slider.y += button_spacing + button_height;


    if( GuiButton( r_button, TextFormat( "Camera Invert X: %s", OptionInverseX() ? "ON" : "OFF" )) ) {
        OptionInverseX( !OptionInverseX() );
    }
    r_slider.y = r_button.y += button_spacing + button_height;

    if( GuiButton( r_button, TextFormat( "Camera Invert Y: %s", OptionInverseY() ? "ON" : "OFF" )) ) {
        OptionInverseY( !OptionInverseY() );
    }
    r_slider.y = r_button.y += button_spacing + button_height;

    float value = OptionVolume();
    if( GuiSlider( r_slider, TextFormat( "Volume %.2f ", value ), "", &value, 0.0, 1.0 )) {
        OptionVolume( value );
    }
    r_button.y = r_slider.y += button_spacing + button_height;

    value = OptionVolumeSFX();
    if( GuiSlider( r_slider, TextFormat( "Volume SFX %.2f ", value ), "", &value, 0.0, 1.0 )) {
        OptionVolumeSFX( value );
    }
    r_button.y = r_slider.y += button_spacing + button_height;

    value = OptionVolumeMusic();
    if( GuiSlider( r_slider, TextFormat( "Volume Music %.2f ", value ), "", &value, 0.0, 1.0 )) {
        OptionVolumeMusic( value );
    }
    r_button.y = r_slider.y += button_spacing + button_height;

    return false;
}
bool draw_credits_menu() {
    Vector2 screen = get_screen();

    Rectangle credits_rect;
    credits_rect.width  = screen.x - (screen.x / 1.4);
    credits_rect.height = screen.y - (screen.y / 1.3);

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
    GuiLabel( right_rect, "3D Art & Animation" );
    right_rect.y = left_rect.y += 24.0;
    group_box_rect.height += 24.0;

    GuiLabel( left_rect, "Clayton Dryden" );
    GuiLabel( right_rect, "Music and Sound Design" );
    right_rect.y = left_rect.y += 24.0;
    group_box_rect.height += 24.0;

    GuiLabel( left_rect, "Jack Ma" );
    GuiLabel( right_rect, "Additional Art" );
    group_box_rect.height += 24.0;

    GuiGroupBox( group_box_rect, "Team" );

    group_box_rect.y      += group_box_rect.height;
    group_box_rect.height  = 0.0;

    // TODO(alicia): additional credits
    return false;
}


