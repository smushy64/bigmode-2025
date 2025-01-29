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

#include "enemy.h"

#include "shared/buffer.h"
#include "audio.h"

#include <string.h>

void spawn_enemy( GlobalState* state, Vector3 position, float rotation, float radius = 10.0f );
void load_sound_set( SoundBuffer* buf, const char* name );
void mode_game_load( GlobalState* state ) {
    auto* game   = &state->transient.game;
    auto* player = &game->player;

    game->camera.position   = {};
    game->camera.projection = CAMERA_PERSPECTIVE;
    game->camera.fovy       = 65.0;
    game->camera.up         = { 0.0, 1.0, 0.0 };

    player->movement_direction = { 0.0, 0.0, 1.0 };
    player->max_velocity       = MAX_WALK_VELOCITY;
    player->max_power = player->power_target = player->power = START_MAX_POWER;

    // game->player_model = LoadModel( "resources/meshes/plug_bot.glb" );
    game->player_model = LoadModel( "resources/meshes/SKM_Plugbot.glb" );

    /* White texture */ {
        Color white = {255, 255, 255, 255};
        Image img;
        img.data    = &white;
        img.width   = 1;
        img.height  = 1;
        img.mipmaps = 1;
        img.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        game->textures.white = LoadTextureFromImage( img );
    }
    load_sound_set( &game->sounds.step, "herostep" );
    load_sound_set( &game->sounds.dash, "herodash" );

    spawn_enemy( state, { 3.0, 0.0, 0.0 }, 0.0 );

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

        case PlayerState::DEFAULT:
        case PlayerState::IS_MOVING:
        case PlayerState::ATTACK_PUNCH:
        case PlayerState::ATTACK_KICK:
        case PlayerState::IS_DEAD:
            break;
    }

    player->power_target    -= damage;
    player->state     = PlayerState::TAKING_DAMAGE;
    player->velocity += impulse;
}

void player_update( GlobalState* state, float dt );
void mode_game_update( GlobalState* state, float dt ) {
    auto* game = &state->transient.game;

    if( IsKeyPressed( KEY_ESCAPE ) ) {
        set_pause( state, !game->is_paused );
    }

    if( !game->is_paused ) {
        player_update( state, dt );
        
        for( int i = 0; i < game->objects.len; ++i ) {
            auto* obj = game->objects.buf;
            if( !obj->is_active ) {
                continue;
            }

            switch( obj->type ) {
                case ObjectType::ENEMY: {
                    obj->enemy.timer += dt;

                    EnemyState start_state = obj->enemy.state;

                    float max_velocity = E_WANDER_MAX_VELOCITY;
                    float drag         = 0.0;
                    switch( obj->enemy.state ) {
                        case EnemyState::IDLE: {
                            drag = 10.0;

                            if( obj->enemy.timer >= E_IDLE_TIME ) {
                                // int chance = GetRandomValue( 0, 1000 );
                                // if( chance > 200 ) {
                                //     obj->enemy.state = EnemyState::SCAN;
                                // } else {
                                //     obj->enemy.state = EnemyState::WANDER;
                                // }
                                // TODO(alicia): 

                                obj->enemy.state = EnemyState::WANDER;
                            }
                        } break;
                        case EnemyState::SCAN: {
                            drag = 10.0;
                            // TODO(alicia): 
                            obj->enemy.state = EnemyState::IDLE;
                        } break;
                        case EnemyState::ALERT: {
                            drag = 10.0;
                        } break;
                        case EnemyState::WANDER: {
                            if( obj->enemy.first_frame_state ) {

                                Vector3 to_home      = obj->enemy.direction_to_home_sqr();
                                float   dist_to_home = Vector3Length( to_home );
                                Vector3 to_target    = obj->enemy.random_direction();

                                if( dist_to_home ) {
                                    to_home /= dist_to_home;
                                }
                                float diff = abs( dist_to_home - obj->enemy.radius );

                                bool flipped = false;
                                float dot = Vector3DotProduct( to_home, to_target );
                                if(
                                    diff < (obj->enemy.radius / 10.0) && dot < 0.0
                                ) {
                                    to_target = Vector3Reflect(
                                        to_target, Vector3Perpendicular( to_target ) );
                                    flipped   = true;
                                }

                                obj->enemy.wander.target = to_target;

                                TraceLog( LOG_INFO, "--------------------------");
                                TraceLog( LOG_INFO, "dist to home:     %f", dist_to_home );
                                TraceLog( LOG_INFO, "diff:             %f", diff );
                                TraceLog( LOG_INFO, "dot:              %f", dot );
                                TraceLog( LOG_INFO, "flipped:          %s", flipped ? "true" : "false" );
                                TraceLog( LOG_INFO, "wander direction: { %f, %f, %f }", to_target.x, to_target.y, to_target.z );
                                TraceLog( LOG_INFO, "--------------------------");

                            }

                            obj->enemy.velocity +=
                                obj->enemy.wander.target * dt * 100.0;

                            if(
                                Vector3LengthSqr( obj->position - obj->enemy.home ) >=
                                (obj->enemy.radius * obj->enemy.radius)
                            ) {
                                obj->enemy.state = EnemyState::RETURN_HOME;
                            } else if( obj->enemy.timer >= E_WANDER_TIME ) {
                                obj->enemy.state = EnemyState::IDLE;
                            }
                        } break;
                        case EnemyState::CHASING: {
                            max_velocity = E_CHASE_MAX_VELOCITY;
                            // TODO(alicia): 
                        } break;
                        case EnemyState::RETURN_HOME: {
                            Vector3 direction = obj->enemy.direction_to_home_sqr();
                            float   distance  = Vector3Length( direction );
                            if( distance ) {
                                direction /= distance;
                            }

                            if( distance < obj->enemy.radius ) {
                                if( distance < E_RETURN_HOME_DISTANCE ) {
                                    obj->enemy.state = EnemyState::IDLE;
                                } else {
                                    int chance = GetRandomValue( 0, 1000 );
                                    if( chance < 500 ) {
                                        obj->enemy.state = EnemyState::IDLE;
                                    }
                                }
                            } else {
                                obj->enemy.velocity +=
                                    direction * dt * 100.0f;
                            }
                        } break;
                    }

                    Vector2 lateral_velocity = { obj->enemy.velocity.x, obj->enemy.velocity.y };
                    lateral_velocity =
                        Vector2ClampValue( lateral_velocity, 0.0, max_velocity );
                    obj->enemy.velocity.x = lateral_velocity.x;
                    obj->enemy.velocity.z = lateral_velocity.y;

                    obj->position       += obj->enemy.velocity * dt;
                    obj->enemy.velocity *= 1.0 - dt * drag;

                    if( start_state != obj->enemy.state ) {
                        obj->enemy.first_frame_state = true;
                        obj->enemy.timer             = 0;
                    } else {
                        obj->enemy.first_frame_state = false;
                    }

                    // TraceLog( LOG_INFO, "%s", to_string( obj->enemy.state ) );
                } break;
                case ObjectType::PLAYER_SPAWN:
                case ObjectType::BATTERY:
                case ObjectType::LEVEL_EXIT: break;

                case ObjectType::NONE:
                case ObjectType::COUNT:      break;
            }
        }
    }

    game_draw( state, dt );
}
void mode_game_unload( GlobalState* state ) {
    auto* game = &state->transient.game;

    if( game->objects.buf ) {
        free( game->objects.buf );
    }

    int sound_buffer_count = sizeof(game->sounds) / sizeof(SoundBuffer);
    for( int i = 0; i < sound_buffer_count; ++i ) {
        SoundBuffer& buffer = ((SoundBuffer*)&game->sounds)[i];
        if( !buffer.buf ) {
            continue;
        }
        for( int j = 0; j < buffer.len; ++j ) {
            UnloadSound( buffer.buf[j] );
        }
        free( buffer.buf );
    }
    memset( &game->sounds, 0, sizeof(game->sounds) );

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

        case PlayerState::ATTACK_PUNCH:
        case PlayerState::ATTACK_KICK:
        case PlayerState::TAKING_DAMAGE:
        case PlayerState::IS_DEAD:
            break;
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

                player->power_target        -= POWER_LOSS_DODGE;
                player->dodge_cooldown_timer = 0.0;

                Vector3 direction_target   = Vector3Normalize( movement );
                player->movement_direction = direction_target;

                play_sfx_random( game->sounds.dash.buf, game->sounds.dash.len );
            }
        } break;

        case PlayerState::ATTACK_PUNCH:
        case PlayerState::ATTACK_KICK:
        case PlayerState::DODGE:
        case PlayerState::TAKING_DAMAGE:
        case PlayerState::IS_DEAD:
            break;
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
            float loss_rate = POWER_LOSS_RATE * POWER_LOSS_IDLE_MULT;
            player->power_target -= loss_rate * dt;

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

            player->sfx_walk_timer += dt;
            if( player->sfx_walk_timer >= player->sfx_walk_time ) {
                if( game->sounds.step.buf ) {
                    int idx = play_sfx_random( game->sounds.step.buf, game->sounds.step.len );

                    player->sfx_walk_time  = sound_length( game->sounds.step.buf[idx] );
                    player->sfx_walk_timer = 0.0;
                }
            }
        } break;
        case PlayerState::DODGE: {
            player->inv_time    += dt;
            player->max_velocity = 100.0;
        } break;
        case PlayerState::TAKING_DAMAGE: {
            player->max_velocity = Lerp( player->max_velocity, 1000.0, dt );
        } break;

        case PlayerState::ATTACK_PUNCH:
        case PlayerState::ATTACK_KICK:
        case PlayerState::IS_DEAD:
            break;
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
    (void)gamepad_is_run_down;
    input->is_run_down = false;

    input->is_dodge_press = gamepad_is_dodge_press || IsKeyPressed( KEY_SPACE );

    input->camera =
        (GetMouseDelta() * ( OptionCameraSensitivity() * 0.003 ) ) +
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

        Matrix transform; {
            Quaternion rot =
                QuaternionFromVector3ToVector3( { 0.0, 0.0, -1.0 }, player->movement_direction );
            transform =
                QuaternionToMatrix( rot ) *
                MatrixTranslate( player->position.x, player->position.y, player->position.z );
        }

        MaterialMap map;
        map.color   = WHITE;
        map.texture = game->textures.white;
        Material mat;
        mat.maps = &map;

        mat.shader = state->sh_basic_shading;

        DrawMesh(
            game->player_model.meshes[0], mat, transform );

        map.color = RED;

        for( int i = 0; i < game->objects.len; ++i ) {
            auto* obj = game->objects.buf + i;
            if( !obj->is_active ) {
                continue;
            }

            switch( obj->type ) {
                case ObjectType::ENEMY: {
                    Quaternion rot =
                        QuaternionFromAxisAngle( { 0.0, 1.0, 0.0 }, obj->rotation );
                    transform =
                        QuaternionToMatrix( rot ) *
                        MatrixTranslate( obj->position.x, obj->position.y, obj->position.z );

                    DrawMesh( game->player_model.meshes[0], mat, transform );

                    DrawCircle3D(
                        obj->enemy.home + Vector3{0.0, 0.1, 0.0},
                        obj->enemy.radius, {1.0, 0.0, 0.0}, 90, GREEN );

                    if( obj->enemy.state == EnemyState::WANDER ) {
                        Vector3 start = obj->position + Vector3{ 0.0, 1.0, 0.0 };
                        Vector3 end   = start + obj->enemy.wander.target;
                        DrawLine3D( start, end, RAYWHITE );
                    }
                } break;

                case ObjectType::PLAYER_SPAWN:
                case ObjectType::BATTERY:     
                case ObjectType::LEVEL_EXIT:  
                case ObjectType::NONE:
                case ObjectType::COUNT: break;
            }
        }

        DrawPlane( {}, { 100.0, 100.0 }, RED );

        EndShaderMode();

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

        static constexpr Color OUTLINE_COLOR  = { 249, 211, 94, 255 };
        static constexpr Color POWER_BG_COLOR = { 50, 20, 71, 255 };
        static constexpr Color POWER_FG_COLOR = { 52, 158, 231, 255 };

        draw( outline       , OUTLINE_COLOR );
        draw( power_box     , POWER_BG_COLOR );
        draw( power_box_fill, POWER_FG_COLOR );

        #undef draw

        Vector2 power_box_center = {
            power_box.x + (power_box.width / 2.0f),
            power_box.y + (power_box.height / 2.0f)
        };
        float power_text_font_size = 24.0;
        const char* power_text =
            TextFormat( "%i / %i", (int)roundf( player->power ), (int)player->max_power );
        Vector2 power_text_size =
            MeasureTextEx( state->persistent.font, power_text, power_text_font_size, 1.0 );
        DrawTextPro(
            state->persistent.font,
            power_text,
            power_box_center,
            power_text_size / 2.0, 0.0, 24.0, 1.0,
            RAYWHITE );

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

#if !defined(RELEASE)
        DrawTextEx(
            state->persistent.font,
            TextFormat( "%i FPS", GetFPS() ),
            {}, 24.0, 1.0, GREEN );
#endif

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
void load_sound_set( SoundBuffer* buf, const char* name ) {
    if( buf->buf ) {
        free( buf->buf );
    }

    int count = 0;
    for( ;; ) {
        const char* path_to_test = TextFormat( "resources/audio/sfx/%s_%i.wav", name, count );
        if( !FileExists( path_to_test ) ) {
            break;
        }
        count++;
    }
    if( !count ) {
        TraceLog( LOG_ERROR, "No sound effects exist with name %s!", name );
        return;
    }

    buf->buf = (Sound*)calloc( count, sizeof(Sound) );
    buf->cap = count;

    for( int i = 0; i < buf->cap; ++i ) {
        Sound sound = LoadSound( TextFormat( "resources/audio/sfx/%s_%i.wav", name, i ) );
        buf_append( buf, sound );
    }
}
void spawn_enemy( GlobalState* state, Vector3 position, float rotation, float radius ) {
    auto* objects = &state->transient.game.objects;
    Object obj = Object::create_enemy( position, rotation, radius );

    if( objects->len ) {
        bool found = false;
        int  index = 0;
        for( int i = 0; i < objects->len; ++i ) {
            if( !objects->buf[i].is_active ) {
                found = true;
                index = i;
                break;
            }
        }

        if( found ) {
            objects->buf[index] = obj;
        } else {
            buf_append( objects, obj );
        }
    } else {
        buf_append( objects, obj );
    }
}
void update( Player* player, float dt ) {
    (void)player;
    (void)dt;
}

