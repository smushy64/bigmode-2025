#if !defined(SHADERS_H)
#define SHADERS_H
/**
 * @file   shaders.h
 * @brief  Shaders.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/

static inline
const char basic_shading_vert[] = R"(
#version 100

/* FROM RAYLIB */

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

/* FROM RAYLIB */

varying vec3 v2f_position;
varying vec2 v2f_uv;
varying vec4 v2f_color;
varying vec3 v2f_normal;

mat3 inverse( mat3 m );
mat3 transpose( mat3 m );

void main() {
    v2f_position = vec3( matModel * vec4( vertexPosition, 1.0 ) );
    v2f_uv       = vertexTexCoord;
    v2f_color    = vertexColor;

    mat3 normal_mat = transpose( inverse( mat3( matModel ) ) );
    v2f_normal      = normalize( normal_mat * vertexNormal );

    gl_Position = mvp * vec4( vertexPosition, 1.0 );
}

mat3 transpose( mat3 m ) {
    return mat3(
        m[0][0], m[1][0], m[2][0],
        m[0][1], m[1][1], m[2][1],
        m[0][2], m[1][2], m[2][2] );
}
mat3 inverse( mat3 m ) {
    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    float b01 =  a22 * a11 - a12 * a21;
    float b11 = -a22 * a10 + a12 * a20;
    float b21 =  a21 * a10 - a11 * a20;

    float det = a00 * b01 + a01 * b11 + a02 * b21;

    return mat3( b01, ( -a22 * a01 + a02 * a21 ), (  a12 * a01 - a02 * a11 ),
                 b11, (  a22 * a00 - a02 * a20 ), ( -a12 * a00 + a02 * a10 ),
                 b21, ( -a21 * a00 + a01 * a20 ), (  a11 * a00 - a01 * a10 ) ) / det;
}

)";

static inline
const char basic_shading_frag[] = R"(
#version 100

precision mediump float;

varying vec3 v2f_position;
varying vec2 v2f_uv;
varying vec4 v2f_color;
varying vec3 v2f_normal;

/* FROM RAYLIB */

uniform sampler2D texture0;

/* FROM RAYLIB */

uniform vec3 camera_position;

void main() {
    vec3 normal      = normalize( v2f_normal );
    vec3 base_color  = texture2D( texture0, v2f_uv ).rgb;
    vec3 from_camera = normalize( camera_position - v2f_position );

    vec3 color = v2f_color.rgb * base_color;

    vec3 ambient = color * vec3( 0.24, 0.24, 0.32 );

    float light_mask = max( dot( from_camera, normal ), 0.0 );

    gl_FragColor = vec4( (color * light_mask) + (ambient * (1.0 - light_mask)), 1.0 );
}

)";

static inline
const char post_process_frag[] = R"(
#version 100

precision mediump float;

/* FROM_RAYLIB */

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4      colDiffuse;

/* FROM_RAYLIB */

uniform vec2 resolution;

vec3 fxaa( sampler2D tex, vec2 uv, vec2 resolution );

void main() {
    vec3 aa_sample = fxaa( texture0, fragTexCoord, resolution ).rgb;
    gl_FragColor   = vec4( aa_sample, 1.0 );
}

vec3 fetch_offset( sampler2D tex, vec2 uv, vec2 pixel_offset, vec2 resolution ) {
    mediump vec2 offset_uv = uv + (pixel_offset / resolution);
    return texture2D( tex, offset_uv ).rgb;
}

#define FXAA_REDUCE_MIN   (1.0/ 128.0)
#define FXAA_REDUCE_MUL   (1.0 / 8.0)
#define FXAA_SPAN_MAX     8.0

vec3 fxaa( sampler2D tex, vec2 uv, vec2 resolution ) {
    vec3 color;

    mediump vec3  rgb_down_left  = fetch_offset( tex, uv, vec2( -1.0, -1.0 ), resolution );
    mediump vec3  rgb_down_right = fetch_offset( tex, uv, vec2(  1.0, -1.0 ), resolution );
    mediump vec3  rgb_up_left    = fetch_offset( tex, uv, vec2( -1.0,  1.0 ), resolution );
    mediump vec3  rgb_up_right   = fetch_offset( tex, uv, vec2(  1.0,  1.0 ), resolution );
    mediump vec3  rgb_m          = texture2D( tex, uv / resolution ).rgb;

    mediump vec3  luma = vec3( 0.299, 0.587, 0.114 );

    mediump float luma_down_left  = dot( rgb_down_left, luma );
    mediump float luma_down_right = dot( rgb_down_right, luma );
    mediump float luma_up_left    = dot( rgb_up_left, luma );
    mediump float luma_up_right   = dot( rgb_up_right, luma );
    mediump float luma_m          = dot( rgb_m, luma );
    mediump float luma_min        = min( luma_m, min( min( luma_down_left, luma_down_right ), min( luma_up_left, luma_up_right ) ) );
    mediump float luma_max        = max( luma_m, max( max( luma_down_left, luma_down_right ), max( luma_up_left, luma_up_right ) ) );

    mediump vec2 dir;
    dir.x = -((luma_down_left + luma_down_right) - (luma_up_left + luma_up_right));
    dir.y =  ((luma_down_left + luma_up_left) - (luma_down_right + luma_up_right));

    float dir_reduce = max( (luma_down_left + luma_down_right + luma_up_left + luma_up_right) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN );

    float rcp_dir_min = 1.0 / ( min( abs(dir.x), abs(dir.y) ) + dir_reduce );

    dir = min( vec2( FXAA_SPAN_MAX ), max( vec2( -FXAA_SPAN_MAX ), dir * rcp_dir_min ) ) / resolution;

    vec3 rgb_a = 0.5 * (
        texture2D( tex, uv + dir * (1.0 / 3.0 - 0.5) ).rgb +
        texture2D( tex, uv + dir * (2.0 / 3.0 - 0.5) ).rgb );

    vec3 rgb_b = rgb_a * 0.5 + 0.25 * (
        texture2D( tex, uv + dir * -0.5 ).rgb +
        texture2D( tex, uv + dir * 0.5 ).rgb );

    float luma_b = dot( rgb_b, luma );

    if( ( luma_b < luma_min ) || ( luma_b > luma_max ) ) {
        color = rgb_a;
    } else {
        color = rgb_b;
    }

    return color;
}

)";

#endif /* header guard */
