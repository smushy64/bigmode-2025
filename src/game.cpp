/**
 * @file   game.cpp
 * @brief  Game mode.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/
#include "modes.h"
#include "state.h"
#include "raylib.h"
#include "raymath.h"
#include "blit.h"

#define CAMERA_DISTANCE 4.0

void mode_game_load( GlobalState* state ) {
    auto* game = &state->transient.game;

    game->camera.position   = {};
    game->camera.projection = CAMERA_PERSPECTIVE;
    game->camera.fovy       = 90.0;
    game->camera.up         = { 0.0, 1.0, 0.0 };
}
void blit_texture( GlobalState* state );
void mode_game_update( GlobalState* state, float dt ) {
    auto* game = &state->transient.game;

    game->player.input.camera = GetMouseDelta();
    game->player.camera_rotation += game->player.input.camera;
    game->player.camera_rotation.y = Clamp( game->player.camera_rotation.y, -80.0, 80.0 );

    Vector3 cam_offset = Vector3{ 0.0, 0.0, CAMERA_DISTANCE }; {
        Quaternion yaw = QuaternionFromAxisAngle(
            Vector3{ 0.0, 1.0, 0.0 },
            game->player.camera_rotation.x * 0.008 );

        Quaternion pitch = QuaternionFromAxisAngle(
            Vector3{ 1.0, 0.0, 0.0 },
            game->player.camera_rotation.y * 0.008 );

        cam_offset = Vector3RotateByQuaternion(
            cam_offset, QuaternionMultiply( yaw, pitch ) );
    }

    game->camera.target   = game->player.position;
    game->camera.position = game->player.position - cam_offset;

    BeginTextureMode( state->rt ); {

        BeginMode3D( game->camera );
        ClearBackground( BLACK );

        DrawCubeV( game->player.position, Vector3One(), WHITE );

        EndMode3D();

    } EndTextureMode();

    BeginDrawing(); {
        blit_texture( state );
        DrawFPS( 0, 0 );
    } EndDrawing();
}
void mode_game_unload( GlobalState* state ) {
    auto* game = &state->transient.game;
    (void)(game);
}

void blit_texture( GlobalState* state ) {
    BeginShaderMode( state->sh_post_process );
        DrawTextureRec(
            state->rt.texture,
            { 0, 0, (float)state->rt.texture.width, -(float)state->rt.texture.height },
            {}, WHITE );
    EndShaderMode();
}

