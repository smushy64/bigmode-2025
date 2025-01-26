/**
 * @file   game.cpp
 * @brief  Game mode.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"

#include "modes.h"
#include "state.h"
#include "blit.h"
#include "globals.h"

#include <string.h>

void mode_game_load( GlobalState* state ) {
    auto* game   = &state->transient.game;
    auto* player = &game->player;

    game->camera.position   = {};
    game->camera.projection = CAMERA_PERSPECTIVE;
    game->camera.fovy       = 90.0;
    game->camera.up         = { 0.0, 1.0, 0.0 };

    player->movement_direction = { 0.0, 0.0, 1.0 };
    player->max_velocity       = MAX_WALK_VELOCITY;
    player->max_power = player->power_target = player->power = START_MAX_POWER;

    DisableCursor();
}
void set_pause( GlobalState* state, bool paused );

void blit_texture( GlobalState* state );
void read_input( GlobalState* state, float dt );
void game_draw( GlobalState* state, float dt );

void player_damage(
    Player* player, Vector3 impulse, float damage
) {
    switch( player->state ) {
        case PlayerState::TAKING_DAMAGE:
        case PlayerState::DODGE:
            return;
    }

    player->power_target    -= damage;
    player->state     = PlayerState::TAKING_DAMAGE;
    player->velocity += impulse;
}

void player_update( GlobalState* state, float dt );
void mode_game_update( GlobalState* state, float dt ) {
    auto* game   = &state->transient.game;

    if( IsKeyPressed( KEY_ESCAPE ) ) {
        set_pause( state, !game->is_paused );
    }

    if( !game->is_paused ) {
        player_update( state, dt );
    }

    game_draw( state, dt );
}
void mode_game_unload( GlobalState* state ) {
    auto* game = &state->transient.game;
    (void)(game);
}
void player_update( GlobalState* state, float dt ) {
    auto* game   = &state->transient.game;
    auto* player = &game->player;

    read_input( state, dt );

    static constexpr float MAX_ROTATION = 80.0 * (1.0 / 180.0);

    player->camera_rotation   += player->input.camera;
    player->camera_rotation.y  = Clamp( player->camera_rotation.y, -MAX_ROTATION, MAX_ROTATION );

    Quaternion yaw = QuaternionFromAxisAngle(
        Vector3{ 0.0, 1.0, 0.0 },
        player->camera_rotation.x );

    Vector3 cam_offset = Vector3{ 0.0, CAMERA_HEIGHT, -CAMERA_DISTANCE }; {
        Quaternion pitch = QuaternionFromAxisAngle(
            Vector3{ 1.0, 0.0, 0.0 },
            player->camera_rotation.y );

        cam_offset = Vector3RotateByQuaternion(
            cam_offset, QuaternionMultiply( yaw, pitch ) );
    }
    game->camera.target   = Vector3Lerp( game->camera.target,   player->position + CAMERA_TARGET_OFFSET, dt * 20.0 );
    game->camera.position = Vector3Lerp( game->camera.position, player->position + cam_offset,           dt * 20.0 );

    Vector3 movement = {};
    movement += Vector3RotateByQuaternion(
        {player->input.movement.x, 0.0, 0.0}, yaw );
    movement += Vector3RotateByQuaternion(
        {0.0, 0.0, player->input.movement.y}, yaw );

    switch( player->state ) {
        case PlayerState::DEFAULT: {
            if( player->input.is_trying_to_move ) {
                player->state = PlayerState::IS_MOVING;
            }
        } break;
        case PlayerState::IS_MOVING: {
            if( !player->input.is_trying_to_move ) {
                player->state = PlayerState::DEFAULT;
            }
        } break;
        case PlayerState::DODGE: {
            if( player->inv_time > DODGE_TIME ) {
                player->state        = PlayerState::DEFAULT;
                player->max_velocity = MAX_WALK_VELOCITY;
                player->inv_time     = 0.0;
            }
        } break;
    }

    switch( player->state ) {
        case PlayerState::DEFAULT:
        case PlayerState::IS_MOVING: {
            if(
                player->input.is_dodge_press &&
                player->dodge_cooldown_timer > DODGE_COOLDOWN_TIME
            ) {
                player->state    = PlayerState::DODGE;
                player->inv_time = 0.0;

                player->velocity = movement * DODGE_SPEED;

                player->power_target               -= DODGE_POWER_LOSS;
                player->dodge_cooldown_timer = 0.0;

                Vector3 direction_target   = Vector3Normalize( movement );
                player->movement_direction = direction_target;
            }
        } break;

        default: break;
    }

    if( player->power <= 0.0 ) {
        player->state    = PlayerState::IS_DEAD;
        player->velocity = {};
        // TODO(alicia): death state
    }

    player->power = fmax( Lerp( player->power, player->power_target, dt * 10.0 ), 0.0 );

    // NOTE(alicia): no more player state changes from here on ---------------

    if( player->state != PlayerState::DODGE ) {
        player->dodge_cooldown_timer += dt;
    }

    float drag = 0.0;
    switch( player->state ) {
        case PlayerState::DEFAULT: {
            player->max_velocity =
                Lerp( player->max_velocity, MAX_WALK_VELOCITY, dt * 10.0 );
            drag = STOP_DRAG;
        } break;
        case PlayerState::IS_MOVING: {
            float loss_rate = POWER_LOSS_RATE *
                ( player->input.is_run_down ? POWER_LOSS_RUNNING_MULT : 1.0 );

            player->power_target -= loss_rate * dt;

            player->max_velocity = Lerp(
                player->max_velocity,
                player->input.is_run_down ? MAX_RUN_VELOCITY : MAX_WALK_VELOCITY, dt * 10.0 );

            player->velocity += movement * dt * 100.0; 

            Vector3 direction_target = Vector3Normalize( movement );
            player->movement_direction =
                Vector3Normalize(
                    Vector3Lerp( player->movement_direction,
                        direction_target, TURN_SPEED * dt ) );
        } break;
        case PlayerState::DODGE: {
            player->inv_time    += dt;
            player->max_velocity = 100.0;
        } break;
        case PlayerState::TAKING_DAMAGE: {
            player->max_velocity = Lerp( player->max_velocity, 1000.0, dt );
        } break;
    }

    switch( player->state ) {
        case PlayerState::DEFAULT:
        case PlayerState::IS_MOVING: {
            Vector2 lateral_velocity = { player->velocity.x, player->velocity.z };
            lateral_velocity =
                Vector2ClampValue( lateral_velocity, 0.0, player->max_velocity );
            player->velocity.x = lateral_velocity.x;
            player->velocity.z = lateral_velocity.y;
        } break;
        default: break;
    }

    player->position += player->velocity * dt;
    player->velocity *= 1.0 - dt * drag;
}

void read_input( GlobalState* state, float dt ) {
    auto* game  = &state->transient.game;
    auto* input = &game->player.input;

    bool    gamepad_is_dodge_press   = false;
    bool    gamepad_is_run_down      = false;
    Vector2 gamepad_stick_right      = {};
    bool    gamepad_stick_left_moved = false;
    Vector2 gamepad_stick_left       = {}; {
        static constexpr float DEADZONE = 0.1;

        if( IsGamepadAvailable( 0 ) ) {

            gamepad_stick_left.x = -GetGamepadAxisMovement( 0, GAMEPAD_AXIS_LEFT_X );
            if( abs( gamepad_stick_left.x ) < DEADZONE ) {
                gamepad_stick_left.x = 0.0;
            }
            gamepad_stick_left.y = -GetGamepadAxisMovement( 0, GAMEPAD_AXIS_LEFT_Y );
            if( abs( gamepad_stick_left.y ) < DEADZONE ) {
                gamepad_stick_left.y = 0.0;
            }

            if( Vector2LengthSqr( gamepad_stick_left ) > 0.01 ) {
                gamepad_stick_left_moved = true;
            }

            gamepad_stick_right.x = GetGamepadAxisMovement( 0, GAMEPAD_AXIS_RIGHT_X );
            if( abs( gamepad_stick_right.x ) < DEADZONE ) {
                gamepad_stick_right.x = 0.0;
            }
            gamepad_stick_right.y = GetGamepadAxisMovement( 0, GAMEPAD_AXIS_RIGHT_Y );
            if( abs( gamepad_stick_right.y ) < DEADZONE ) {
                gamepad_stick_right.y = 0.0;
            }

            gamepad_is_run_down    = IsGamepadButtonDown( 0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT );
            gamepad_is_dodge_press = IsGamepadButtonPressed( 0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT );
        }
    }

    int horizontal = (int)IsKeyDown( KEY_A ) - (int)IsKeyDown( KEY_D );
    int vertical   = (int)IsKeyDown( KEY_W ) - (int)IsKeyDown( KEY_S );

    input->is_trying_to_move = horizontal || vertical || gamepad_stick_left_moved;
    input->movement =
        Vector2Normalize({ (float)horizontal, (float)vertical }) + gamepad_stick_left;

    input->movement = Vector2ClampValue( input->movement, 0.0, 1.0 );

    // input->is_run_down = gamepad_is_run_down || IsKeyDown( KEY_LEFT_SHIFT );
    input->is_run_down = false;

    input->is_dodge_press = gamepad_is_dodge_press || IsKeyPressed( KEY_SPACE );

    input->camera =
        (GetMouseDelta() * ( OptionCameraSensitivity() * 0.005 ) ) +
        (gamepad_stick_right * (OptionCameraSensitivity() * 3.0 * dt));

    if( OptionInverseX() ) {
        input->camera.x = -input->camera.x;
    }
    if( OptionInverseY() ) {
        input->camera.y = -input->camera.y;
    }
}
void game_draw( GlobalState* state, float dt ) {
    auto* game   = &state->transient.game;
    auto* player = &game->player;

    BeginDrawing();

    if( OptionFXAA() ) {
        BeginTextureMode( state->rt );
    }

    /* 3D */ {

        BeginMode3D( game->camera );
        BeginShaderMode( state->sh_basic_shading );

        ClearBackground( BLUE );

        SetShaderValue(
            state->sh_basic_shading,
            state->sh_basic_shading_loc_camera_position,
            &game->camera.position, SHADER_UNIFORM_VEC3 );

        Vector3 direction_pointer_offset = { 0.0, 1.0, 0.5 }; {
            Quaternion rot = QuaternionFromVector3ToVector3(
                { 0.0, 0.0, 1.0 }, player->movement_direction );
            direction_pointer_offset =
                Vector3RotateByQuaternion( direction_pointer_offset, rot );
        }

        DrawCubeV( player->position + direction_pointer_offset, {0.25, 0.25, 0.25}, YELLOW );

        DrawPlane( {}, { 100.0, 100.0 }, RED );

        EndShaderMode();

        float radius        = 0.5;
        float player_height = 2.0;
        DrawCapsule(
            player->position + Vector3{ 0.0, radius, 0.0 },
            player->position + Vector3{ 0.0, player_height - radius, 0.0 },
            radius, 16, 16, WHITE );

        EndMode3D();

    }

    if( OptionFXAA() ) {
        EndTextureMode();
    }

    /* UI */ {
        if( OptionFXAA() ) {
            blit_texture( state );
        }

        Rectangle power_box = {
            30.0, 30.0,
            player->max_power,
            28.0
        };

        Rectangle outline = power_box;
        outline.y = outline.x -= 2.0;
        outline.width  += 4.0;
        outline.height += 4.0;

        Rectangle power_box_fill = power_box;
        power_box_fill.width = (player->power / player->max_power) * player->max_power;

        power_box_fill.x = power_box_fill.y += 2.0;
        power_box_fill.width  -= 4.0;
        power_box_fill.height -= 4.0;

        #define draw( rect, color ) \
            DrawRectangleRounded( rect, 0.6, 4, color )

        draw( outline, WHITE );
        draw( power_box, BLACK );
        draw( power_box_fill, (Color{ 0, 0, 255, 255 }) );

        #undef draw

        Vector2 power_box_center = {
            power_box.x + (power_box.width / 2.0f),
            power_box.y + (power_box.height / 2.0f)
        };
        float power_text_font_size = 24.0;
        const char* power_text =
            TextFormat( "%i / %i", (int)player->power, (int)player->max_power );
        Vector2 power_text_size =
            MeasureTextEx( state->persistent.font, power_text, power_text_font_size, 1.0 );
        DrawTextPro(
            state->persistent.font,
            power_text,
            power_box_center,
            power_text_size / 2.0, 0.0, 24.0, 1.0, WHITE );

        if( game->is_paused ) {
            if( !draw_pause_menu( game->pause_menu_state ) ) {
                set_pause( state, false );
            }

            if( game->pause_menu_state.quit_to_menu ) {
                mode_set( state, Mode::MAIN_MENU );
                EndDrawing();
                return;
            }
        }

        DrawTextEx(
            state->persistent.font, TextFormat( "%i FPS", GetFPS() ), {}, 24.0, 1.0, GREEN );
    }

    EndDrawing();
}

void blit_texture( GlobalState* state ) {
    BeginShaderMode( state->sh_post_process );
        DrawTextureRec(
            state->rt.texture,
            { 0, 0, (float)state->rt.texture.width, -(float)state->rt.texture.height },
            {}, WHITE );
    EndShaderMode();
}
void set_pause( GlobalState* state, bool paused ) {
    auto* game = &state->transient.game;
    game->is_paused = paused;

    if( paused ) {
        TraceLog( LOG_INFO, "Game paused." );
        EnableCursor();
        memset( &game->player.input, 0, sizeof(game->player.input) );
        game->pause_menu_state.is_options_open = false;
    } else {
        TraceLog( LOG_INFO, "Game unpaused." );
        DisableCursor();
    }
}

