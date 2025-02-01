/**
 * @file   game.cpp
 * @brief  Game mode.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
// IWYU pragma: begin_keep
#include "raylib.h"
#include "raymath.h"
#include "raygui.h"
#include "rlgl.h"

#include "modes.h"
#include "state.h"
#include "blit.h"
#include "globals.h"

#include "enemy.h"

#include "shared/buffer.h"
#include "shared/world.h"
#include "audio.h"

#include <string.h>
// IWYU pragma: end_keep

/// @brief Color code black.
#define ANSI_COLOR_BLACK   "\033[1;30m"
/// @brief Color code white.
#define ANSI_COLOR_WHITE   "\033[1;37m"
/// @brief Color code red.
#define ANSI_COLOR_RED     "\033[1;31m"
/// @brief Color code green.
#define ANSI_COLOR_GREEN   "\033[1;32m"
/// @brief Color code blue.
#define ANSI_COLOR_BLUE    "\033[1;34m"
/// @brief Color code magenta.
#define ANSI_COLOR_MAGENTA "\033[1;35m"
/// @brief Color code yellow.
#define ANSI_COLOR_YELLOW  "\033[1;33m"
/// @brief Color code cyan.
#define ANSI_COLOR_CYAN    "\033[1;36m"
/// @brief Color code to reset color.
#define ANSI_COLOR_RESET   "\033[1;00m"

void DrawPlane( Material mat, Vector2 texture_tile, Vector3 centerPos, Vector2 size, Color color );
void DrawPlaneInv( Material mat, Vector2 texture_tile, Vector3 centerPos, Vector2 size, Color color );

Vector2 world_collision_check(
    int segment_count, Segment* segments, Vector2* vertexes,
    Vector2 position, Vector2 velocity, float radius = 1.0 );

void spawn_enemy(
    GlobalState* state, Vector3 position,
    float rotation, float radius = E_DEFAULT_RADIUS );
void load_sound_set( SoundBuffer* buf, const char* name );

void load_map( GlobalState* state, const char* path );

void player_init( Player* player ) {
    player->movement_direction = { 0.0, 0.0, 1.0 };
    player->max_velocity       = MAX_WALK_VELOCITY;
    player->max_power = player->power_target = player->power = START_MAX_POWER;
}
void camera_init( Camera3D* camera ) {
    *camera = Camera3D{};

    camera->projection = CAMERA_PERSPECTIVE;
    camera->fovy       = 65.0;
    camera->up         = { 0.0, 1.0, 0.0 };
}

enum class Animation {
    BIND_POSE,
    DAMAGED,
    DEATH,
    DODGE_DIVE,
    DODGE_STATIONARY,
    IDLE,
    KICK01,
    PUNCH01,
    PUNCH02,
    RUN,
    WALK,

    COUNT
};
const char* ANIMATION_NAMES[] = {
    "BindPose",
    "Damaged",
    "Death",
    "Dodge_Dive",
    "Dodge_Stationary",
    "Idle",
    "Kick_01",
    "Punch_01",
    "Punch_02",
    "Run",
    "Walk",
};
int ANIMATION_INDEXES[(int)Animation::COUNT] = {};

#define TraceLog( ... )

extern const char* INITIAL_MAP;
void mode_game_load( GlobalState* state ) {
    auto* game = &state->transient.game;

    game->textures.test = LoadTexture( "resources/textures/test.jpg" );

    #define PLAYER_MODEL_PATH "resources/meshes/SKM_Plugbot.glb"

    game->models.player = LoadModel( PLAYER_MODEL_PATH );
    game->player_animation.buf = LoadModelAnimations(
        PLAYER_MODEL_PATH, &game->player_animation.len );

    game->models.wall = LoadModel( "resources/meshes/scene_wall.glb" );

    /* Animations */ {
        TraceLog( LOG_INFO, "Animations: ");
        auto* animations = game->player_animation.buf;
        int   count      = game->player_animation.len;
        int   running_index = 0;
        for( int i = 0; i < count; ++i ) {
            auto* a = animations + i;
            for( int j = 0; j < (int)Animation::COUNT; ++j ) {
                if( strcmp( ANIMATION_NAMES[j], a->name ) == 0 ) {
                    ANIMATION_INDEXES[running_index++] = i;
                    break;
                }
            }
            TraceLog( LOG_INFO, "    %i: %s", i, a->name );
            TraceLog( LOG_INFO, "        frame count: %i", a->frameCount );

            if( !IsModelAnimationValid( game->models.player, *a ) ) {
                TraceLog( LOG_ERROR, "NOT VALID!" );
                abort();
            }
        }
    }

    TraceLog( LOG_INFO, "Loading %s . . .", INITIAL_MAP );
    load_map( state, INITIAL_MAP );

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
    load_sound_set( &game->sounds.step,  "herostep" );
    load_sound_set( &game->sounds.dash,  "steamherodash" );
    load_sound_set( &game->sounds.whiff, "whiff" );
    load_sound_set( &game->sounds.punch, "punch" );

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
        case PlayerState::ATTACK:
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

                    Vector3 current_direction;
                    float   velocity_length_sqr = Vector3LengthSqr( obj->enemy.velocity );
                    if( !velocity_length_sqr ) {
                        current_direction = obj->enemy.facing_direction;
                    } else {
                        current_direction = obj->enemy.velocity / sqrt( velocity_length_sqr );
                    }

                    EnemyState start_state = obj->enemy.state;

                    Vector2 scan_direction;

                    float max_velocity = E_WANDER_MAX_VELOCITY;
                    float drag         = 0.0;
                    switch( obj->enemy.state ) {
                        case EnemyState::IDLE: {
                            drag = 10.0;

                            if( obj->enemy.timer >= E_IDLE_TIME ) {
                                int lo  = 0;
                                int hi  = 1000;

                                int chance = GetRandomValue( lo, hi );
                                (void)chance;

                                if( chance > 250 ) {
                                    obj->enemy.state = EnemyState::WANDER;
                                } else {
                                    obj->enemy.state = EnemyState::SCAN;
                                }
                            }
                        } break;
                        case EnemyState::SCAN: {
                            drag = 10.0;

                            Vector3 start_direction = obj->enemy.facing_direction;
                            Vector3 end_direction   = start_direction;
                            start_direction = Vector3RotateByAxisAngle(
                                start_direction, Vector3UnitY, 45 * (180.0 / M_PI) );
                            end_direction = Vector3RotateByAxisAngle(
                                end_direction, Vector3UnitY, -45 * (180.0 / M_PI) );

                            float t = obj->enemy.timer / E_SCAN_TIME;
                            Vector3 scan_direction3 =
                                -Vector3Normalize(
                                    Vector3Lerp( start_direction, end_direction, t ) );

                            scan_direction = { scan_direction3.x, scan_direction3.z };

                            if( obj->enemy.timer >= E_SCAN_TIME ) {
                                int lo = 0;
                                int hi = 1000;
                                int chance = GetRandomValue( lo, hi );

                                if( chance > 250 ) {
                                    obj->enemy.state = EnemyState::WANDER;
                                } else {
                                    obj->enemy.state = EnemyState::IDLE;
                                }
                            }
                        } break;
                        case EnemyState::ALERT: {
                            drag = 10.0;

                            if( obj->enemy.timer >= E_ALERT_TIME ) {
                                obj->enemy.state = EnemyState::CHASING;
                            }
                        } break;
                        case EnemyState::WANDER: {
                            if( obj->enemy.first_frame_state ) {

                                float rotation =
                                    (float)GetRandomValue( 0, 360 ) * (M_PI / 180.0);
                                Vector3 to_target =
                                    Vector3RotateByAxisAngle(
                                        obj->enemy.facing_direction, Vector3UnitY, rotation );

                                Vector3 to_home      = obj->enemy.direction_to_home_sqr();
                                float   dist_to_home = Vector3Length( to_home );
                                if( dist_to_home ) {
                                    to_home /= dist_to_home;
                                }
                                float diff = abs( dist_to_home - obj->enemy.radius );
                                if(
                                    diff < (obj->enemy.radius / 8.0) &&
                                    Vector3DotProduct( to_home, to_target ) < 0.0
                                ) {
                                    to_target = Vector3Reflect( to_target, -to_target );
                                }
                                obj->enemy.wander.direction = to_target;

                            }

                            obj->enemy.velocity +=
                                obj->enemy.wander.direction * dt * E_ACCELERATION;

                            if(
                                Vector3LengthSqr( obj->position - obj->enemy.home ) >=
                                (obj->enemy.radius * obj->enemy.radius)
                            ) {
                                obj->enemy.state = EnemyState::RETURN_HOME;
                            } else if( obj->enemy.timer >= E_WANDER_TIME ) {
                                obj->enemy.state = EnemyState::IDLE;
                            }

                            obj->enemy.sfx_timer += dt;
                            if( obj->enemy.sfx_timer >= E_SFX_WALK_TIME ) {
                                obj->enemy.sfx_timer = 0.0;
                                play_sfx_random(
                                    { game->player.position.x, game->player.position.z },
                                    { obj->position.x, obj->position.z },
                                    game->sounds.step.buf, game->sounds.step.len );
                            }

                        } break;
                        case EnemyState::CHASING: {
                            max_velocity = E_CHASE_MAX_VELOCITY;

                            Vector3 direction =
                                Vector3Normalize( game->player.position - obj->position );

                            float dist_sqr = Vector3DistanceSqr( obj->position, game->player.position );
                            if( dist_sqr >= PLAYER_COLLISION_RADIUS_2 * 2.0 ) {
                                obj->enemy.velocity +=
                                    direction * dt * E_CHASE_ACCELERATION;
                            }

                            if(
                                Vector3LengthSqr( obj->position - obj->enemy.home ) >=
                                (obj->enemy.radius * obj->enemy.radius)
                            ) {
                                obj->enemy.state = EnemyState::RETURN_HOME;
                            }
                            if(
                                Vector3LengthSqr( obj->position - game->player.position ) <
                                PLAYER_COLLISION_RADIUS_2
                            ) {
                                obj->enemy.state = EnemyState::ATTACKING;
                                play_sfx_random(
                                    { game->player.position.x, game->player.position.z },
                                    { obj->position.x, obj->position.z },
                                    game->sounds.whiff.buf, game->sounds.whiff.len );
                            }

                            obj->enemy.sfx_timer += dt;
                            if( obj->enemy.sfx_timer >= E_SFX_RUN_TIME ) {
                                obj->enemy.sfx_timer = 0.0;
                                play_sfx_random(
                                    { game->player.position.x, game->player.position.z },
                                    { obj->position.x, obj->position.z },
                                    game->sounds.step.buf, game->sounds.step.len );
                            }
                        } break;
                        case EnemyState::ATTACKING: {
                            drag = 10.0;

                            Vector2 attack_circle = 
                                Vector2{obj->position.x, obj->position.z} +
                                Vector2{obj->enemy.facing_direction.x,
                                    obj->enemy.facing_direction.z};
                            Vector2 player_circle =
                                Vector2{ game->player.position.x, game->player.position.z };

                            if(
                                game->player.state != PlayerState::DODGE         &&
                                game->player.state != PlayerState::TAKING_DAMAGE &&
                                game->player.state != PlayerState::IS_DEAD       &&
                                CheckCollisionCircles(
                                attack_circle, 1.0,
                                player_circle, PLAYER_COLLISION_RADIUS
                            ) ) {
                                game->player.power_target -= E_ATTACK_POWER;

                                game->player.velocity +=
                                    obj->enemy.facing_direction * E_ATTACK_PUSH;

                                game->player.state = PlayerState::TAKING_DAMAGE;

                                play_sfx_random(
                                    { game->camera.position.x, game->camera.position.z },
                                    { obj->position.x, obj->position.z },
                                    game->sounds.punch.buf,
                                    game->sounds.punch.len );
                            } else if( obj->enemy.timer >= E_ATTACK_TIME ) {
                                obj->enemy.state = EnemyState::CHASING;
                            }
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
                                    if( chance < 400 ) {
                                        obj->enemy.state = EnemyState::IDLE;
                                    }
                                }
                            } else {
                                obj->enemy.velocity +=
                                    direction * dt * E_ACCELERATION;
                            }

                            obj->enemy.sfx_timer += dt;
                            if( obj->enemy.sfx_timer >= E_SFX_WALK_TIME ) {
                                obj->enemy.sfx_timer = 0.0;
                                play_sfx_random(
                                    { game->player.position.x, game->player.position.z },
                                    { obj->position.x, obj->position.z },
                                    game->sounds.step.buf, game->sounds.step.len );
                            }

                        } break;
                        case EnemyState::TAKING_DAMAGE: {
                            max_velocity = 1000.0;
                            if( obj->enemy.timer > E_TAKING_DAMAGE_TIME ) {
                                obj->enemy.state = EnemyState::CHASING;
                            }
                        } break;
                        case EnemyState::DYING: {
                            drag = 100.0;
                            if( obj->enemy.timer > E_DYING_TIME + 0.2 ) {
                                obj->is_active = false;
                            }
                        } break;
                    }

                    switch( obj->enemy.state ) {
                        case EnemyState::IDLE:
                        case EnemyState::SCAN:
                        case EnemyState::WANDER:
                        case EnemyState::ALERT:
                        case EnemyState::CHASING:
                        case EnemyState::RETURN_HOME: {
                            auto* player = &game->player;
                            if( player->state == PlayerState::ATTACK ) {
                                Vector2 player_attack_position =
                                    Vector2{ player->position.x, player->position.z } +
                                    (Vector2{
                                        player->movement_direction.x,
                                        player->movement_direction.z
                                    } * 2.0);

                                Vector2 pos = { obj->position.x, obj->position.z };

                                if( CheckCollisionCircles(
                                    pos, 1.0, player_attack_position, 1.0
                                ) ) {
                                    obj->enemy.state = EnemyState::TAKING_DAMAGE;
                                    obj->enemy.power -= ATTACK_DAMAGE;

                                    Vector3 to_player =
                                        Vector3Normalize( obj->position - player->position );

                                    drag         = 0.0;
                                    max_velocity = 1000.0;
                                    obj->enemy.velocity += to_player * E_ATTACK_PUSH;

                                    if( obj->enemy.power < 0.0 ) {
                                        obj->enemy.state = EnemyState::DYING;
                                    }

                                    play_sfx_random(
                                        { game->camera.position.x, game->camera.position.z },
                                        { obj->position.x, obj->position.z },
                                        game->sounds.punch.buf,
                                        game->sounds.punch.len );
                                }
                            }
                        } break;

                        case EnemyState::ATTACKING:
                        case EnemyState::TAKING_DAMAGE:
                        case EnemyState::DYING: break;
                    }

                    obj->enemy.facing_direction = Vector3Lerp(
                        obj->enemy.facing_direction,
                        current_direction, dt * 10.0
                    ); {
                        Vector2 lateral_velocity = 
                            { obj->enemy.velocity.x, obj->enemy.velocity.z };
                        lateral_velocity =
                            Vector2ClampValue( lateral_velocity, 0.0, max_velocity );
                        obj->enemy.velocity.x = lateral_velocity.x;
                        obj->enemy.velocity.z = lateral_velocity.y;
                    }

                    Vector2 sight_start, sight_end;
                    switch( obj->enemy.state ) {
                        case EnemyState::SCAN: {
                            sight_start = { obj->position.x, obj->position.z };
                            sight_end   = sight_start + scan_direction * E_SIGHT_RANGE;
                        } break;
                        case EnemyState::IDLE:
                        case EnemyState::WANDER:
                        case EnemyState::RETURN_HOME: {
                            sight_start = { obj->position.x, obj->position.z };
                            sight_end   = sight_start +
                                Vector2{ current_direction.x, current_direction.z } *
                                E_SIGHT_RANGE;
                        } break;
                        case EnemyState::TAKING_DAMAGE:
                        case EnemyState::DYING:
                        case EnemyState::ATTACKING:
                        case EnemyState::ALERT:
                        case EnemyState::CHASING:
                            break;
                    }

                    bool wall_blocked = false;

                    Vector3 velocity = obj->enemy.velocity; {
                        Vector2 position = { obj->position.x, obj->position.z };
                        float speed = Vector3Length( velocity );

                        for( int j = 0; j < game->segments.len; ++j ) {
                            Segment* seg = game->segments.buf + j;
                            Vector2  start, end;

                            start = game->vertexes.buf[seg->start];
                            end   = game->vertexes.buf[seg->end];

                            if( CheckCollisionCircleLine( position, 1.0, start, end ) ) {
                                Vector2 normal = Vector2Rotate(
                                    Vector2Normalize( start - end ), 90 * (M_PI / 180.0) );

                                Vector2 center    = Vector2Lerp( start, end, 0.5 );
                                Vector2 to_object = Vector2Normalize( position - center );

                                if( Vector2DotProduct( normal, to_object ) < 0.0 ) {
                                    normal = -normal;
                                }

                                // NOTE(alicia): cancel movement towards collision
                                velocity += Vector3{ normal.x, 0, normal.y } * speed;
                            }

                            if(
                                !wall_blocked &&
                                obj->enemy.state != EnemyState::ALERT &&
                                obj->enemy.state != EnemyState::CHASING
                            ) {
                                Vector2 block_point = {};
                                bool wall_collision = CheckCollisionLines(
                                    start, end, sight_start, sight_end, &block_point );
                                if( wall_collision ) {
                                    sight_end    = block_point;
                                    wall_blocked = true;
                                }
                            }

                        }

                    }

                    if(
                        game->player.state != PlayerState::IS_DEAD    &&
                        obj->enemy.state != EnemyState::ALERT         &&
                        obj->enemy.state != EnemyState::CHASING       &&
                        obj->enemy.state != EnemyState::ATTACKING     &&
                        obj->enemy.state != EnemyState::TAKING_DAMAGE &&
                        obj->enemy.state != EnemyState::DYING         &&
                        obj->enemy.state != EnemyState::RETURN_HOME
                    ) {
                        if( CheckCollisionCircleLine(
                            {game->player.position.x, game->player.position.z},
                            PLAYER_COLLISION_RADIUS, sight_start, sight_end 
                        ) ) {
                            obj->enemy.state = EnemyState::ALERT;
                        }
                    }

                    obj->position       += velocity * dt;
                    obj->enemy.velocity *= 1.0 - dt * drag;

                    if( start_state != obj->enemy.state ) {
                        obj->enemy.first_frame_state = true;
                        obj->enemy.timer             = 0;
                        obj->enemy.sfx_timer         = 0;
                    } else {
                        obj->enemy.first_frame_state = false;
                    }

                    obj->position.y = 0.0;
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
    if( game->vertexes.buf ) {
        free( game->vertexes.buf );
    }
    if( game->segments.buf ) {
        free( game->segments.buf );
    }

    int texture_count = sizeof(game->textures) / sizeof(Texture);
    for( int i = 0; i < texture_count; ++i ) {
        Texture* texture = (Texture*)(&game->textures) + i;
        UnloadTexture( *texture );
    }

    UnloadModelAnimations( game->player_animation.buf, game->player_animation.len );
    game->player_animation.len = 0;
    
    int model_count = sizeof(game->models) / sizeof(Model);
    for( int i = 0; i < model_count; ++i ) {
        Model* model = (Model*)(&game->models) + i;
        UnloadModel( *model );
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

    PlayerState start_state = player->state;

    player->camera_rotation   += player->input.camera;
    player->camera_rotation.y  = Clamp(
        player->camera_rotation.y, CAMERA_MIN_ROTATION, CAMERA_MAX_ROTATION);

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
    game->camera.target   = Vector3Lerp(
        game->camera.target,
        player->position + CAMERA_TARGET_OFFSET,
        dt * 20.0 );
    game->camera.position = Vector3Lerp(
        game->camera.position,
        player->position + cam_offset,
        dt * 20.0 );

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

        case PlayerState::ATTACK: {
            if( player->attack_timer > ATTACK_TIME ) {
                player->state        = PlayerState::DEFAULT;
                player->attack_timer = 0.0;
                player->max_velocity = MAX_WALK_VELOCITY;
            }
        } break;

        case PlayerState::TAKING_DAMAGE: {
            if( player->inv_time > TAKING_DAMAGE_TIME ) {
                player->state        = PlayerState::DEFAULT;
                player->max_velocity = MAX_WALK_VELOCITY;
                player->inv_time     = 0.0;
            }
        } break;

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

                play_sfx_random( 
                    {}, {},
                    game->sounds.dash.buf, game->sounds.dash.len );

            } else if( player->input.is_punch_press ) {
                player->state         = PlayerState::ATTACK;
                player->power_target -= POWER_LOSS_ATTACK;

                player->which_attack = !player->which_attack;

                play_sfx_random(
                    {}, {},
                    game->sounds.whiff.buf,
                    game->sounds.whiff.len );
            }
        } break;

        case PlayerState::ATTACK:
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
                    int idx = play_sfx_random(
                        {}, {},
                        game->sounds.step.buf, game->sounds.step.len );

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
            player->inv_time     += dt;
            player->max_velocity  = Lerp( player->max_velocity, 1000.0, dt );
        } break;

        case PlayerState::ATTACK: {
            player->attack_timer += dt;
            drag = STOP_DRAG;
        } break;
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

    Vector3 velocity; {
        Vector2 v2 = world_collision_check(
            game->segments.len, game->segments.buf, game->vertexes.buf,
            { player->position.x, player->position.z },
            { player->velocity.x, player->velocity.z },
            PLAYER_COLLISION_RADIUS );

        velocity = { v2.x, player->velocity.y, v2.y };
    }
    player->position += velocity * dt;

    player->velocity *= 1.0 - dt * drag;

    if( start_state != player->state ) {
        player->animation_frame = 0;
        player->animation_timer = 0;
    }
}

float filter_deadzone( float raw, float deadzone ) {
    if( abs( raw ) > deadzone ) {
        return raw;
    } else {
        return 0.0;
    }
}
Vector2 filter_deadzone( Vector2 raw, Vector2 deadzones ) {
    Vector2 result;
    if( abs( raw.x ) > deadzones.x ) {
        result.x = raw.x;
    } else {
        result.x = 0.0;
    }
    if( abs( raw.y ) > deadzones.y ) {
        result.y = raw.y;
    } else {
        result.y = 0.0;
    }
    return result;
}
void read_input( GlobalState* state, float dt ) {
    auto* game  = &state->transient.game;
    auto* input = &game->player.input;

    bool lmb = IsMouseButtonPressed( MOUSE_BUTTON_LEFT );

    bool switched_to_keyboard = false;
    if( input->is_using_gamepad ) {
        int anykey = GetKeyPressed();
        int x, y; {
            Vector2 delta = GetMouseDelta();
            x = delta.x;
            y = delta.y;
        }
        if( anykey || x || y || lmb ) {
            input->is_using_gamepad = false;
            switched_to_keyboard    = true;
        }
    }

    bool    gamepad_is_dodge_press   = false;
    bool    gamepad_is_run_down      = false;
    bool    gamepad_is_punch_press   = false;
    Vector2 gamepad_stick_right      = {};
    bool    gamepad_stick_left_moved = false;
    Vector2 gamepad_stick_left       = {}; {
        static constexpr float DEADZONE = 0.1;

        if( IsGamepadAvailable( 0 ) ) {
            Vector2 stick_left = {
                GetGamepadAxisMovement( 0, GAMEPAD_AXIS_LEFT_X ),
                GetGamepadAxisMovement( 0, GAMEPAD_AXIS_LEFT_Y )
            };

            Vector2 stick_right = {
                GetGamepadAxisMovement( 0, GAMEPAD_AXIS_RIGHT_X ),
                GetGamepadAxisMovement( 0, GAMEPAD_AXIS_RIGHT_Y )
            };

            stick_left  = filter_deadzone( stick_left, { DEADZONE, DEADZONE } );
            stick_right = filter_deadzone( stick_right, { DEADZONE, DEADZONE } );

            float trigger_left  = GetGamepadAxisMovement( 0, GAMEPAD_AXIS_LEFT_TRIGGER );
            float trigger_right = GetGamepadAxisMovement( 0, GAMEPAD_AXIS_RIGHT_TRIGGER );

            trigger_left  = filter_deadzone(
                Remap( trigger_left, -1.0, 1.0, 0.0, 1.0 ), DEADZONE );
            trigger_right = filter_deadzone(
                Remap( trigger_right, -1.0, 1.0, 0.0, 1.0 ), DEADZONE );

            if( !input->is_using_gamepad && !switched_to_keyboard ) {
                int anybutton = GetGamepadButtonPressed();

                if(
                    anybutton     ||
                    stick_left.x  ||
                    stick_left.y  ||
                    stick_right.x ||
                    stick_right.y ||
                    trigger_left  ||
                    trigger_right
                ) {
                    input->is_using_gamepad = true;
                }
            }

            gamepad_stick_left  = -stick_left;
            gamepad_stick_right = stick_right;

            if( Vector2LengthSqr( stick_left ) > 0.01 ) {
                gamepad_stick_left_moved = true;
            }

            gamepad_is_run_down    =
                IsGamepadButtonDown( 0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT );
            gamepad_is_dodge_press =
                IsGamepadButtonPressed( 0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN );
            gamepad_is_punch_press =
                IsGamepadButtonPressed( 0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT );
        }
    }

    int horizontal = (int)IsKeyDown( KEY_A ) - (int)IsKeyDown( KEY_D );
    int vertical   = (int)IsKeyDown( KEY_W ) - (int)IsKeyDown( KEY_S );

    input->is_trying_to_move = horizontal || vertical || gamepad_stick_left_moved;
    input->movement =
        Vector2Normalize({ (float)horizontal, (float)vertical }) + gamepad_stick_left;

    input->movement = Vector2ClampValue( input->movement, 0.0, 1.0 );

    (void)gamepad_is_run_down;
    input->is_run_down = false;

    input->is_dodge_press = gamepad_is_dodge_press || IsKeyPressed( KEY_SPACE );
    input->is_punch_press = gamepad_is_punch_press || lmb;

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
        ClearBackground( BLUE );

        BeginShaderMode( state->sh_basic_shading );
        // NOTE(alicia): BEGIN SHADER

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

        float animation_speed = 1.0;
        ModelAnimation* anim = 0;
        switch( player->state ) {
            case PlayerState::DEFAULT: {
                anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::IDLE];
            } break;
            case PlayerState::IS_MOVING: {
                anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::RUN];
            } break;
            case PlayerState::ATTACK: {
                int which = (int)(player->which_attack ? Animation::PUNCH01 : Animation::KICK01);
                anim = game->player_animation.buf + ANIMATION_INDEXES[which];
            } break;
            case PlayerState::DODGE: {
                anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::DODGE_DIVE];
                animation_speed = 1.9;
            } break;
            case PlayerState::TAKING_DAMAGE: {
                anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::DAMAGED];
            } break;
            case PlayerState::IS_DEAD: {
                anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::DEATH];
            } break;
        }

        UpdateModelAnimation(
            game->models.player, *anim, player->animation_frame % anim->frameCount );
        if( player->animation_timer >= ANIMATION_TIME ) {
            if( !(
                player->state == PlayerState::IS_DEAD &&
                player->animation_frame >= anim->frameCount - 1
            ) ) {
                player->animation_frame++;
                player->animation_timer = 0.0;
            }
        }
        player->animation_timer += dt * animation_speed;

        DrawMesh(
            game->models.player.meshes[0], mat, transform );
        map.color = RED;

        for( int i = 0; i < game->objects.len; ++i ) {
            auto* obj = game->objects.buf + i;
            if( !obj->is_active ) {
                continue;
            }

            switch( obj->type ) {
                case ObjectType::ENEMY: {
                    Quaternion rot =
                        QuaternionFromVector3ToVector3(
                            { 0.0, 0.0, -1.0 }, obj->enemy.facing_direction );
                    transform =
                        QuaternionToMatrix( rot ) *
                        MatrixTranslate( obj->position.x, obj->position.y, obj->position.z );

                    switch( obj->enemy.state ) {
                        case EnemyState::IDLE: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::IDLE];
                        } break;
                        case EnemyState::SCAN: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::IDLE];
                        } break;
                        case EnemyState::WANDER: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::WALK];
                        } break;
                        case EnemyState::ALERT: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::IDLE];
                        } break;
                        case EnemyState::CHASING: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::RUN];
                        } break;
                        case EnemyState::ATTACKING: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::PUNCH02];
                        } break;
                        case EnemyState::RETURN_HOME: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::WALK];
                        } break;
                        case EnemyState::TAKING_DAMAGE: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::DAMAGED];
                        } break;
                        case EnemyState::DYING: {
                            anim = game->player_animation.buf + ANIMATION_INDEXES[(int)Animation::DEATH];
                        } break;
                    }

                    UpdateModelAnimation(
                        game->models.player, *anim,
                        obj->enemy.animation_frame % anim->frameCount );

                    if( obj->enemy.animation_timer >= ANIMATION_TIME ) {
                        if( !(
                            obj->enemy.state == EnemyState::DYING &&
                            obj->enemy.animation_frame >= anim->frameCount - 1
                        ) ) {
                            obj->enemy.animation_frame++;
                            obj->enemy.animation_timer = 0.0;
                        }
                    }
                    obj->enemy.animation_timer += dt;

                    DrawMesh( game->models.player.meshes[0], mat, transform );
                } break;

                case ObjectType::PLAYER_SPAWN:
                case ObjectType::BATTERY:     
                case ObjectType::LEVEL_EXIT:  
                case ObjectType::NONE:
                case ObjectType::COUNT: break;
            }
        }

        SetMaterialTexture( &mat, MATERIAL_MAP_DIFFUSE, game->textures.test );
        /* Draw Ceiling/Floor */ {
            DrawPlaneInv( mat, {1000, 1000}, { 0.0,  10.1, 0.0 }, { 10000.0, 10000.0 }, WHITE );
            DrawPlane   ( mat, {1000, 1000}, { 0.0,  -0.1, 0.0 }, { 10000.0, 10000.0 }, WHITE );
        }
        /* Draw Walls */ {
            auto* vert = &game->vertexes;
            auto* seg  = &game->segments;

            map.color = RAYWHITE;
            for( int s = 0; s < seg->len; ++s ) {
                Vector2 start, end;
                start = vert->buf[seg->buf[s].start];
                end   = vert->buf[seg->buf[s].end];

                float angle = Vector2Angle( start - end, Vector2{ 0.0, 1.0 });
                float dist  = Vector2Distance( start, end );

                Matrix transform =
                    MatrixScale( 0.1, 10.0, dist ) *
                    MatrixRotateY( angle ) *
                    MatrixTranslate( start.x, 0.0, start.y );
                DrawMesh( game->models.wall.meshes[0], mat, transform );
            }
        }

        // NOTE(alicia): END SHADER
        EndShaderMode();

// NOTE(alicia): DEBUG DRAWING
#if 0
        DrawCylinderWires(
            player->position, 1.0, 1.0, 2.0, 8, CYAN );
        if( player->state == PlayerState::ATTACK ) {
            DrawCylinderWires(
                player->position + (player->movement_direction * 2.0), 
                1.0, 1.0, 2.0, 8, RED );
        }


        for( int i = 0; i < game->objects.len; ++i ) {
            auto* obj = game->objects.buf + i;
            if( !obj->is_active ) {
                continue;
            }

            switch( obj->type ) {
                case ObjectType::ENEMY: {
                    DrawCircle3D(
                        obj->enemy.home + Vector3{0.0, 0.1, 0.0},
                        obj->enemy.radius, {1.0, 0.0, 0.0}, 90, GREEN );

                    float cylinder_thickness = 0.01;

                    Vector3 start = obj->position + Vector3UnitY;

                    DrawCylinderEx(
                        start, start + (obj->enemy.facing_direction * E_SIGHT_RANGE),
                        cylinder_thickness, cylinder_thickness, 8, GOLD );

                    switch( obj->enemy.state ) {
                        case EnemyState::WANDER: {
                            Vector3 end = start + (obj->enemy.wander.direction * 2.0);
                            DrawCylinderEx(
                                start, end, cylinder_thickness,
                                cylinder_thickness, 8, WHITE );
                        } break;
                        case EnemyState::SCAN: {
                            Vector3 start_direction = -obj->enemy.facing_direction;
                            Vector3 end_direction   = start_direction;
                            start_direction = Vector3RotateByAxisAngle(
                                start_direction, Vector3UnitY, 45 * (180.0 / M_PI) );
                            end_direction = Vector3RotateByAxisAngle(
                                end_direction, Vector3UnitY, -45 * (180.0 / M_PI) );

                            float t = obj->enemy.timer / E_SCAN_TIME;
                            Vector3 current_direction =
                                Vector3Normalize(
                                    Vector3Lerp( start_direction, end_direction, t ) );

                            Vector3 end;
                            end = start_direction * E_SIGHT_RANGE;
                            DrawCylinderEx(
                                start, start + end,
                                cylinder_thickness, cylinder_thickness, 8, WHITE );
                            end = current_direction * E_SIGHT_RANGE;
                            DrawCylinderEx(
                                start, start + end,
                                cylinder_thickness, cylinder_thickness, 8, GOLD );
                            end = end_direction * E_SIGHT_RANGE;
                            DrawCylinderEx(
                                start, start + end,
                                cylinder_thickness, cylinder_thickness, 8, WHITE );

                        } break;

                        case EnemyState::ALERT: {
                        } break;
                        case EnemyState::ATTACKING: {
                            DrawCylinderWires(
                                obj->position + obj->enemy.facing_direction,
                                1.0, 1.0, 2.0, 8, RED );
                        } break;

                        case EnemyState::TAKING_DAMAGE:
                        case EnemyState::DYING:
                        case EnemyState::IDLE:
                        case EnemyState::CHASING:
                        case EnemyState::RETURN_HOME: break;
                    }

                } break;
                case ObjectType::NONE:
                case ObjectType::PLAYER_SPAWN:
                case ObjectType::BATTERY:
                case ObjectType::LEVEL_EXIT:
                case ObjectType::COUNT: break;
            }
        }
#endif

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
void load_map( GlobalState* state, const char* path ) {
    auto* game = &state->transient.game;

    player_init( &game->player );
    camera_init( &game->camera );

    int size = 0;
    unsigned char* data = LoadFileData( path, &size );

    MapFileHeader* header = (MapFileHeader*)data;
    if(
        ( size < (int)sizeof(*header) ) ||
        ( memcmp( header->identifier, MAP_IDENTIFIER, 4 ) != 0 ) ||
        ( size != (int)header->total_size )
    ) {
        TraceLog( LOG_ERROR, "%s is an invalid file!", path );
        UnloadFileData( data );
        return;
    }

    auto* st = game;
    st->objects.len  = 0;
    st->segments.len = 0;
    st->vertexes.len = 0;

    MapFileObject*  obj  = (MapFileObject*)(header + 1);
    Vector2*        vert = (Vector2*)(obj + header->object_count);
    MapFileSegment* seg  = (MapFileSegment*)(vert + header->vertex_count);

    if( st->objects.cap < header->object_count) {
        st->objects.buf = (Object*)realloc(
            st->objects.buf, sizeof(Object) * header->object_count );
    }
    if( st->vertexes.cap < header->vertex_count ) {
        st->vertexes.buf = (Vector2*)realloc(
            st->vertexes.buf, sizeof(Vector2) * header->vertex_count );
    }
    if( st->segments.cap < header->segment_count ) {
        st->segments.buf = (Segment*)realloc(
            st->segments.buf, sizeof(Segment) * header->segment_count );
    }

    for( uint16_t i = 0; i < header->object_count; ++i ) {
        auto* o = obj + i;
        switch( o->type ) {
            case ObjectType::ENEMY: {
                spawn_enemy(
                    state, { o->position.x, 0.0, o->position.y },
                    o->rotation, E_DEFAULT_RADIUS );
            } break;

            case ObjectType::PLAYER_SPAWN: {
                game->player.position = { o->position.x, 0, o->position.y };
            } break;

            case ObjectType::BATTERY:
            case ObjectType::LEVEL_EXIT:
            case ObjectType::NONE:
            case ObjectType::COUNT: break;
        }
    }

    memcpy( st->vertexes.buf, vert, sizeof(Vector2) * header->vertex_count );
    st->vertexes.len = header->vertex_count;

    for( uint16_t i = 0; i < header->segment_count; ++i ) {
        Segment s = {};
        s.start = seg[i].start;
        s.end   = seg[i].end;
        buf_append( &st->segments, s );
    }
    UnloadFileData( data );
    TraceLog( LOG_INFO, "Loaded %s!", path );
}
void DrawPlane(
    Material mat, Vector2 texture_tile, Vector3 centerPos,
    Vector2 size, Color color
) {
    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlSetTexture( mat.maps->texture.id );
        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlNormal3f(0.0f, 1.0f, 0.0f);

            rlTexCoord2f( 0.0, 0.0 );
            rlVertex3f( -0.5f, 0.0f, -0.5f );

            rlTexCoord2f( 0.0, texture_tile.y );
            rlVertex3f( -0.5f, 0.0f,  0.5f );

            rlTexCoord2f( texture_tile.x, texture_tile.y );
            rlVertex3f(  0.5f, 0.0f,  0.5f );

            rlTexCoord2f( texture_tile.x, 0.0 );
            rlVertex3f(  0.5f, 0.0f, -0.5f );

            // rlVertex3f(  0.5f, 0.0f,  0.5f );
        rlEnd();
        rlSetTexture(0);
    rlPopMatrix();
}
void DrawPlaneInv(
    Material mat, Vector2 texture_tile, Vector3 centerPos,
    Vector2 size, Color color
) {
    // NOTE: Plane is always created on XZ ground
    rlPushMatrix();
        rlTranslatef(centerPos.x, centerPos.y, centerPos.z);
        rlScalef(size.x, 1.0f, size.y);

        rlSetTexture( mat.maps->texture.id );
        rlBegin(RL_QUADS);
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlNormal3f(0.0f, -1.0f, 0.0f);

            rlTexCoord2f( texture_tile.x, texture_tile.y );
            rlVertex3f( -0.5f, 0.0f,  0.5f );

            rlTexCoord2f( texture_tile.x, 0.0 );
            rlVertex3f( -0.5f, 0.0f, -0.5f );

            rlTexCoord2f( 0.0, 0.0 );
            rlVertex3f(  0.5f, 0.0f, -0.5f );

            rlTexCoord2f( 0.0, texture_tile.y );
            rlVertex3f(  0.5f, 0.0f,  0.5f );
        rlEnd();
        rlSetTexture(0);
    rlPopMatrix();
}
Vector2 world_collision_check(
    int segment_count, Segment* segments, Vector2* vertexes,
    Vector2 position, Vector2 velocity, float radius
) {
    float speed = Vector2Length( velocity );
    for( int i = 0; i < segment_count; ++i ) {
        Segment* seg = segments + i;
        Vector2  start, end;

        start = vertexes[seg->start];
        end   = vertexes[seg->end];

        if( CheckCollisionCircleLine( position, radius, start, end ) ) {
            Vector2 normal = Vector2Rotate(
                Vector2Normalize( start - end ), 90 * (M_PI / 180.0) );

            Vector2 center    = Vector2Lerp( start, end, 0.5 );
            Vector2 to_object = Vector2Normalize( position - center );

            if( Vector2DotProduct( normal, to_object ) < 0.0 ) {
                normal = -normal;
            }

            // NOTE(alicia): cancel movement towards collision
            velocity += normal * speed;
        }
    }

    return velocity;
}

