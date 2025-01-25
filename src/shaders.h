#if !defined(SHADERS_H)
#define SHADERS_H
/**
 * @file   shaders.h
 * @brief  Shaders.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   January 24, 2025
*/

static inline
const char post_process_frag[] = R"(
#version 100

precision mediump float;

varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform sampler2D texture0;
uniform vec4      colDiffuse;
uniform vec2      resolution;

vec3 fxaa( sampler2D src_tex, vec2 uv, vec2 resolution );

// out vec4 out_color;
void main() {
    // vec2 resolution = vec2(textureSize( texture0, 0 ));
    vec3 aa_sample = fxaa( texture0, fragTexCoord, resolution ).rgb;
    // out_color = vec4( aa_sample, 1.0 );
    gl_FragColor = vec4( aa_sample, 1.0 );
}

float rgb_to_luma( vec3 rgb ) {
    return sqrt( dot( rgb, vec3( 0.299, 0.587, 0.114 ) ) );
}

#define FXAA_EDGE_THRESHOLD_MIN 0.0312
#define FXAA_EDGE_THRESHOLD_MAX 0.125
#define FXAA_QUALITY( q ) ( (q) < 5 ? 1.0 : ( (q) > 5 ? ( (q) < 10 ? 2.0 : ( (q) < 11 ? 4.0 : 8.0 ) ) : 1.5) )
#define FXAA_ITERATIONS (14)

vec3 fetch_offset( sampler2D tex, vec2 uv, vec2 pixel_offset, vec2 resolution ) {
    vec2 offset_uv = uv + (pixel_offset / resolution);
    return texture2D( tex, offset_uv ).rgb;
}

vec3 fxaa( sampler2D src_tex, vec2 uv, vec2 resolution ) {
    vec3 render_sample = texture2D( src_tex, uv ).rgb;

    float luma_center = rgb_to_luma( render_sample );

    float luma_down  = rgb_to_luma( fetch_offset( src_tex, uv, vec2( 0, -1), resolution ) );
    float luma_up    = rgb_to_luma( fetch_offset( src_tex, uv, vec2( 0,  1), resolution ) );
    float luma_left  = rgb_to_luma( fetch_offset( src_tex, uv, vec2(-1,  0), resolution ) );
    float luma_right = rgb_to_luma( fetch_offset( src_tex, uv, vec2( 1,  0), resolution ) );

    float luma_min = min( luma_center, min( min( luma_down, luma_up ), min( luma_left, luma_right ) ) );
    float luma_max = max( luma_center, max( max( luma_down, luma_up ), max( luma_left, luma_right ) ) );

    float luma_range = luma_max - luma_min;

    // if not on edge, don't AA
    if( luma_range < max( FXAA_EDGE_THRESHOLD_MIN, luma_max * FXAA_EDGE_THRESHOLD_MAX ) ) {
        return render_sample;
    }

    float luma_down_left =
        rgb_to_luma( fetch_offset( src_tex, uv, vec2(-1,-1), resolution ) );
    float luma_down_right =
        rgb_to_luma( fetch_offset( src_tex, uv, vec2( 1,-1), resolution ) );
    float luma_up_left =
        rgb_to_luma( fetch_offset( src_tex, uv, vec2(-1, 1), resolution ) );
    float luma_up_right =
        rgb_to_luma( fetch_offset( src_tex, uv, vec2( 1, 1), resolution ) );

    float luma_down_up    = luma_down + luma_up;
    float luma_left_right = luma_left + luma_right;

    float luma_left_corner  = luma_down_left  + luma_up_left;
    float luma_down_corner  = luma_down_left  + luma_down_right;
    float luma_right_corner = luma_down_right + luma_up_right;
    float luma_up_corner    = luma_up_left    + luma_up_right;

    float edge_horizontal =
        abs( -2.0 * luma_left + luma_left_corner ) +
        abs( -2.0 * luma_center + luma_down_up )
        * 2.0 + abs( -2.0 * luma_right * luma_right_corner );

    float edge_vertical =
        abs( -2.0 * luma_up + luma_up_corner ) +
        abs( -2.0 * luma_center + luma_left_right )
        * 2.0 + abs( -2.0 * luma_down + luma_down_corner );

    bool is_horizontal = edge_horizontal > edge_vertical;

    float luma1 = is_horizontal ? luma_down : luma_left;
    float luma2 = is_horizontal ? luma_up : luma_right;

    float gradient1 = luma1 - luma_center;
    float gradient2 = luma2 - luma_center;

    bool is_1_steepest = abs(gradient1) >= abs(gradient2);

    float gradient_scaled = 0.25 * max( abs( gradient1 ), abs( gradient2 ) );

    vec2 inverse_screen_size = vec2( 1.0 / resolution.x, 1.0 / resolution.y );

    float step_length = is_horizontal ? inverse_screen_size.y : inverse_screen_size.x;

    float luma_local_average = 0.0;

    if( is_1_steepest ) {
        step_length        = -step_length;
        luma_local_average = 0.5 * ( luma1 + luma_center );
    } else {
        luma_local_average = 0.5 * ( luma2 + luma_center );
    }

    vec2 current_uv = uv;
    if( is_horizontal ) {
        current_uv.y += step_length * 0.5;
    } else {
        current_uv.x += step_length * 0.5;
    }

    vec2 offset = is_horizontal ? vec2( inverse_screen_size.x, 0.0 ) : vec2( 0.0, inverse_screen_size.y );

    vec2 uv1 = current_uv - offset;
    vec2 uv2 = current_uv + offset;

    float luma_end1 = rgb_to_luma( texture2D( src_tex, uv1 ).rgb );
    float luma_end2 = rgb_to_luma( texture2D( src_tex, uv2 ).rgb );
    luma_end1 -= luma_local_average;
    luma_end2 -= luma_local_average;

    bool reached1 = abs( luma_end1 ) >= gradient_scaled;
    bool reached2 = abs( luma_end2 ) >= gradient_scaled;
    bool reached_both = reached1 && reached2;

    if( !reached1 ) {
        uv1 -= offset;
    }
    if( !reached2 ) {
        uv2 += offset;
    }

    if( !reached_both ) {
        for( int i = 2; i < FXAA_ITERATIONS; ++i ) {
            if( !reached1 ) {
                luma_end1 = rgb_to_luma( texture2D( src_tex, uv1 ).rgb );
                luma_end1 = luma_end1 - luma_local_average;
            }

            if( !reached2 ) {
                luma_end2 = rgb_to_luma( texture2D( src_tex, uv2 ).rgb );
                luma_end2 = luma_end2 - luma_local_average;
            }

            reached1 = abs( luma_end1 ) >= gradient_scaled;
            reached2 = abs( luma_end2 ) >= gradient_scaled;
            reached_both = reached1 && reached2;

            if( !reached1 ) {
                uv1 -= offset * FXAA_QUALITY(i);
            }
            if( !reached2 ) {
                uv2 += offset * FXAA_QUALITY(i);
            }

            if( reached_both ) {
                break;
            }
        }
    }

    float distance1 = is_horizontal ? ( uv.x - uv1.x ) : ( uv.y - uv1.y );
    float distance2 = is_horizontal ? ( uv2.x - uv.x ) : ( uv2.y - uv.y );

    bool is_direction1   = distance1 < distance2;
    float distance_final = min( distance1, distance2 );

    float edge_thickness = distance1 + distance2;

    bool is_luma_center_smaller = luma_center < luma_local_average;

    bool correct_variation1 = ( luma_end1 < 0.0 ) != is_luma_center_smaller;
    bool correct_variation2 = ( luma_end2 < 0.0 ) != is_luma_center_smaller;

    bool correct_variation = is_direction1 ? correct_variation1 : correct_variation2;

    float pixel_offset = -distance_final / edge_thickness + 0.5;
    float final_offset = correct_variation ? pixel_offset : 0.0;

    float luma_average = ( 1.0 / 12.0 ) * ( 2.0 * ( luma_down_up + luma_left_right ) + luma_left_corner + luma_right_corner );

    float sub_pixel_offset_1 = clamp( abs( luma_average - luma_center ) / luma_range, 0.0, 1.0 );
    float sub_pixel_offset_2 = ( -2.0 * sub_pixel_offset_1 + 3.0 ) * sub_pixel_offset_1 * sub_pixel_offset_1;

    float sub_pixel_offset_final = sub_pixel_offset_2 * sub_pixel_offset_2 * 0.75;

    final_offset = max( final_offset, sub_pixel_offset_final );

    vec2 final_uv = uv;
    if( is_horizontal ) {
        final_uv.y += final_offset * step_length;
    } else {
        final_uv.x += final_offset * step_length;
    }

    vec3 final_color = texture2D( src_tex, final_uv ).rgb;
    return final_color;
}

)";

#endif /* header guard */
