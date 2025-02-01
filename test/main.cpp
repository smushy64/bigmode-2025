/**
 * @file   main.cpp
 * @brief  Quick test-bed. Linux only.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 31, 2025
*/
#include "raylib.h"
#include "raymath.h"

int main() {
    InitWindow( 800, 600, "Test" );
    SetTargetFPS( 60 );

    Camera3D camera = {};
    camera.up         = Vector3UnitY;
    camera.fovy       = 60;
    camera.target     = Vector3UnitY;
    camera.position   = (-Vector3UnitZ * 3.0) + Vector3UnitY;
    camera.projection = CAMERA_PERSPECTIVE;

    #define path "resources/meshes/Anim_Plugbot_Walk_2.glb"
    Model model = LoadModel( path );
    int count = 0;
    ModelAnimation* anim = LoadModelAnimations( path, &count );

    if( !IsModelAnimationValid( model, anim[0]) ) {
        abort();
    }

    float timer = 0;
    int frame = 0;
    while( !WindowShouldClose() ) {
        UpdateModelAnimation( model, anim[0], frame );

        BeginDrawing();
        ClearBackground( BLACK );

        DrawText( TextFormat( "%i / %i", frame, anim[0].frameCount ), 0, 0, 24, RED );
        BeginMode3D( camera );

            DrawModel( model, {}, 1.0, WHITE );

            timer += GetFrameTime();
            if( timer >= 0.1 ) {
                frame = (frame + 1) % anim[0].frameCount;
            }

        EndMode3D();

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
