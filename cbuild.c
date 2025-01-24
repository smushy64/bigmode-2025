/**
 * @file   cbuild.c
 * @brief  Build system for GMTK 2024.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   August 14, 2024
*/
#include "cbuild.h"
#include <unistd.h>

#define GAME_NAME "bigmode-2025"
// 1 gib
#define TOTAL_MEMORY 536870912

#define COMMON_RAYLIB_ARGS \
    "-c", "-Wno-missing-braces", "-Werror=pointer-arith", "-fno-strict-aliasing", \
    "-std=c99", "-O1", "-Wall", "-Werror=implicit-function-declaration", \
    "-Iraylib/src", "-Iraylib/src/external/glfw/include"

#define WINDOWS_RAYLIB_ARGS \
    COMMON_RAYLIB_ARGS, \
    "-DPLATFORM_DESKTOP_GLFW", "-DGRAPHICS_API_OPENGL_33" \

#define GNU_LINUX_RAYLIB_ARGS \
    COMMON_RAYLIB_ARGS, "-D_GNU_SOURCE", \
    "-DPLATFORM_DESKTOP_GLFW", "-DGRAPHICS_API_OPENGL_33", \
    "-D_GLFW_X11"

#define WEB_RAYLIB_ARGS \
    "-c", "-Os", "-Wall", \
    "-D_GNU_SOURCE", "-DPLATFORM_WEB", "-DGRAPHICS_API_OPENGL_ES2", \
    "-std=gnu99", "-Iraylib/src", "-Iraylib/src/external/glfw/include", "-fPIC"

#define COMMON_ARGS \
    "src/main.cpp", "-Isrc", "-Iraylib/src", "-Wall"

#define WINDOWS_ARGS \
    COMMON_ARGS, "-Lvendor/windows", "-l:libraylib.a", "-static-libgcc", "-static", "-lgdi32", "-lwinmm", \
    "-lopengl32", "-lshell32", "-o", "build/windows/" GAME_NAME ".exe"

#define GNU_LINUX_ARGS \
    COMMON_ARGS, "-Lvendor/linux", "-l:libraylib.a", \
    "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11", "-static-libgcc", \
    "-o", "build/linux/" GAME_NAME

#define WEB_ARGS \
    COMMON_ARGS, "vendor/web/libraylib.a", \
    "-Lvendor/web/libraylib.a", "-sUSE_GLFW=3", \
    "--shell-file", "raylib/src/minshell.html", \
    "-sTOTAL_MEMORY=" macro_value_to_string(TOTAL_MEMORY), \
    "--preload-file", "resources", \
    "-o", "build/web/" GAME_NAME ".html", "-DPLATFORM_WEB", \
    "-sERROR_ON_UNDEFINED_SYMBOLS=0"

#if defined(PLATFORM_WINDOWS)
    #define EXE_EXT ".exe"
#else
    #define EXE_EXT
#endif

enum Target {
    T_NATIVE,
    T_GNU_LINUX,
    T_WINDOWS,
    T_WEB,

    T_COUNT
};
String target_to_string( enum Target target );
String target_compiler( enum Target target );
String target_cpp_compiler( enum Target target );
String target_ar( enum Target target );
bool   target_parse( String src, enum Target* out_target );
enum Target target_native(void);
bool target_is_native( enum Target target );

enum Mode {
    M_HELP,
    M_BUILD,
    M_RUN,
    M_PACKAGE,

    M_COUNT
};
String mode_to_string( enum Mode mode );
String mode_description( enum Mode mode );
bool   mode_parse( String src, enum Mode* out_mode );

struct Args {
    enum Mode mode;
    union {
        struct Help {
            enum Mode mode;
        } help;
        struct Build {
            enum Target target    : 4;
            bool is_strip_symbols : 1;
            bool is_optimized     : 1;
        } build;
        struct Run {
            struct Build build;
        } run;
        struct Package {
            struct Build build;
        } package;
    };
};

int mode_help( struct Args* args );
int mode_build( struct Args* args );
int mode_run( struct Args* args );
int mode_package( struct Args* args );

bool __make_dirs( const char* first, ... );
#define make_dirs( ... ) __make_dirs( __VA_ARGS__, NULL )

int __quick_cmd( Command _cmd );
#define quick_cmd( ... ) __quick_cmd( command_new( __VA_ARGS__ ) )

int main( int argc, const char** argv ) {
    init( LOGGER_LEVEL_INFO );
    argc--; argv++;
    struct Args args = {};

    if( !argc ) {
        return mode_help( &args );
    }
    args.mode = M_COUNT;

    for( int i = 0; i < argc; ++i ) {
        String arg = string_from_cstr( argv[i] );

        if( args.mode == M_COUNT ) {
            if( !mode_parse( arg, &args.mode ) ) {
                fprintf( stderr, "error: unrecognized mode '%s'!\n", arg.cc );
                args.mode = M_HELP;
                mode_help( &args );
                return 1;
            }
            continue;
        }

        if( args.mode == M_HELP ) {
            if( !mode_parse( arg, &args.help.mode ) ) {
                fprintf( stderr, "error: unrecognized mode '%s'!\n", arg.cc );
                args.mode = M_HELP;
                mode_help( &args );
                return 1;
            }
            break;
        } else {
            if( args.mode == M_BUILD || args.mode == M_RUN ) {
                if( string_cmp( arg, string_text("-optimized") ) ) {
                    args.build.is_optimized = true;
                    continue;
                }
                if( string_cmp( arg, string_text("-strip-symbols") ) ) {
                    args.build.is_strip_symbols = true;
                    continue;
                }
            }

            if( args.mode == M_BUILD ) {
                String target_prefix = string_text("-target=");
                if(
                    arg.len >= target_prefix.len && 
                    string_cmp( string_new( target_prefix.len, arg.cc ), target_prefix )
                ) {
                    String target_to_parse = string_advance_by( arg, target_prefix.len );
                    enum Target t;
                    if( target_parse( target_to_parse, &t ) ) {
                        args.build.target = t;
                        continue;
                    }
                }
            }
        }

        fprintf( stderr, "error: unrecognized flag '%s'\n", arg.cc );
        mode_help( &args );
        return 1;
    }

    if( args.mode != M_HELP && args.build.target == T_NATIVE ) {
        args.build.target = target_native();
    }

    switch( args.mode ) {
        case M_HELP:    return mode_help( &args );
        case M_BUILD:   return mode_build( &args );
        case M_RUN:     return mode_run( &args );
        case M_PACKAGE: return mode_package( &args );
        case M_COUNT:   return 1;
    }

    return 0;
}

int build_dependency_raylib(
    enum Target target, const char* cc, const char* ar );
int build_linux( const char* cpp, struct Build* build );
int build_windows( const char* cpp, struct Build* build );
int build_web( const char* cpp, struct Build* build );
int mode_build( struct Args* args ) {
    f64 start = timer_milliseconds();

    struct Build* build = &args->build;

    String cc  = target_compiler( build->target );
    String cpp = target_cpp_compiler( build->target );
    String ar  = target_ar( build->target );

    if( !process_in_path( cc.cc ) ) {
        cb_error( "%s is required!", cc.cc );
        return 1;
    }
    if( !process_in_path( cpp.cc ) ) {
        cb_error( "%s is required!", cpp.cc );
        return 1;
    }
    if( !process_in_path( ar.cc ) ) {
        cb_error( "%s is required!", ar.cc );
        return 1;
    }

    cb_info( "Building for target %s . . .", target_to_string( build->target ).cc );
    cb_info( "C   compiler: %s", cc.cc );
    cb_info( "C++ compiler: %s", cpp.cc );
    cb_info( "AR:           %s", ar.cc );

    make_dirs( "vendor", local_fmt( "vendor/%s", target_to_string(build->target).cc ) );

    if( !path_exists(
        local_fmt("vendor/%s/libraylib.a", target_to_string(build->target).cc)
    ) ) {
        int result = build_dependency_raylib(
            build->target, cc.cc, ar.cc );
        if( result ) {
            return result;
        }
    }

    int result = 0;
    switch( build->target ) {
        case T_GNU_LINUX: {
            result = build_linux( cpp.cc, build );
        } break;
        case T_WINDOWS: {
            result = build_windows( cpp.cc, build );
        } break;
        case T_WEB: {
            result = build_web( cpp.cc, build );
        } break;

        case T_NATIVE:
        case T_COUNT:  unreachable();
    }

    if( result ) {
        return result;
    }

    f64 end = timer_milliseconds();
    cb_info( "Build took %fms", end - start );
    return 0;
}
int mode_run( struct Args* args ) {
    if( !target_is_native( args->build.target ) ) {
        cb_error( "mode 'run' can only be used with native target!" );
        return 1;
    }

    int result = mode_build( args );
    if( result ) {
        return result;
    }

    const char* name = local_fmt(
        "build/%s/" GAME_NAME EXE_EXT, target_to_string(args->build.target).cc );
    cb_info( "Running with command %s . . .", name );

    Command cmd = command_new( name );
    PID pid = process_exec( cmd, false, 0, 0, 0, 0 );
    result  = process_wait( pid );
    if( result ) {
        cb_info( "Process exited abnormally with code %i", result );
    }

    return 0;
}
int mode_package( struct Args* args ) {

    if( !process_in_path( "zip" ) ) {
        cb_error( "zip is required!" );
        return 1;
    }

    if( path_exists( "build" ) ) {
        dir_remove( "build", true );
    }

    memory_zero( &args->build, sizeof(args->build) );
    int result  = 0;

#if defined(PLATFORM_LINUX)
    args->build.is_optimized = args->build.is_strip_symbols = true;

    args->build.target = T_GNU_LINUX;
    result             = mode_build( args );
    if( result ) {
        return result;
    }
#else
    cb_warn( "Skipping linux version . . ." );
#endif

    args->build.target = T_WINDOWS;
    result             = mode_build( args );
    if( result ) {
        return result;
    }

    args->build.target = T_WEB;
    result             = mode_build( args );
    if( result ) {
        return result;
    }

    result = quick_cmd( "zip", "-r", "resources.zip", "resources" );
    if( result ) {
        cb_error( "Failed to zip resources!" );
        return result;
    }

#if defined(PLATFORM_LINUX)
    if( !file_copy( "build/linux/bigmode-2025-linux-x86-64.zip", "resources.zip" )) {
        cb_error( "Failed to copy zipped resources!" );
        return 1;
    }
    if( !file_move( "build/windows/bigmode-2025-win32-x86-64.zip", "resources.zip" )) {
        cb_error( "Failed to move zipped resources!" );
        return 1;
    }
#else
    if( !file_move( "build/windows/bigmode-2025-win32-x86-64.zip", "resources.zip" )) {
        cb_error( "Failed to move zipped resources!" );
        return 1;
    }
#endif

#if defined(PLATFORM_LINUX)
    chdir( "build/linux" );
    result = quick_cmd( "zip", "bigmode-2025-linux-x86-64.zip", GAME_NAME );
    if( result ) {
        cb_error( "Failed to zip linux version!" );
        chdir( "../.." );
        return result;
    }

    chdir( "../windows" );

    result = quick_cmd( "zip", "bigmode-2025-win32-x86-64.zip", GAME_NAME ".exe" );
    chdir( "../.." );
    if( result ) {
        cb_error( "Failed to zip windows version!" );
        return result;
    }

#else
    chdir( "build/windows" );

    result = quick_cmd( "zip", "bigmode-2025-win32-x86-64.zip", GAME_NAME ".exe" );
    chdir( "../.." );
    if( result ) {
        cb_error( "Failed to zip windows version!" );
        return result;
    }
#endif

    chdir( "build/web" );
    result = quick_cmd( "zip", "-r", "bigmode-2025-web.zip", "." );
    chdir( "../.." );
    if( result ) {
        cb_error( "Failed to zip web version!" );
        return result;
    }

    return 0;
}

int build_linux( const char* cpp, struct Build* build ) {
    make_dirs( "build", "build/linux" );

    CommandBuilder builder;
    command_builder_new( cpp, &builder );
    command_builder_append( &builder, GNU_LINUX_ARGS );

    if( build->is_optimized ) {
        command_builder_push( &builder, "-O2" );
    }
    if( !build->is_strip_symbols ) {
        command_builder_push( &builder, "-ggdb" );
    }

    Command cmd = command_builder_cmd( &builder );

    PID pid = process_exec( cmd, false, 0, 0, 0, 0 );

    command_builder_free( &builder );

    return process_wait( pid );
}
int build_windows( const char* cpp, struct Build* build ) {
    make_dirs( "build", "build/windows" );

    CommandBuilder builder;
    command_builder_new( cpp, &builder );
    command_builder_append( &builder, WINDOWS_ARGS );

    if( build->is_optimized ) {
        command_builder_push( &builder, "-O2" );
    }
    if( !build->is_strip_symbols ) {
        command_builder_append( &builder, "-g" );
    }

    Command cmd = command_builder_cmd( &builder );

    PID pid = process_exec( cmd, false, 0, 0, 0, 0 );

    command_builder_free( &builder );

    return process_wait( pid );
}
int build_web( const char* cpp, struct Build* build ) {
    make_dirs( "build", "build/web" );

    CommandBuilder builder;
    command_builder_new( cpp, &builder );
    command_builder_append( &builder, WEB_ARGS );

    if( !build->is_strip_symbols ) {
        command_builder_push( &builder, "--profiling" );
    }

    Command cmd = command_builder_cmd( &builder );

    PID pid = process_exec( cmd, false, 0, 0, 0, 0 );

    command_builder_free( &builder );

    int result = process_wait( pid );
    if( result ) {
        return result;
    }

    if( path_exists( "build/web/index.html" ) ) {
        if( !file_remove( "build/web/index.html" ) ) {
            cb_error( "Failed to remove stale index.html!" );
            return 1;
        }
    }

    if( !file_move( "build/web/index.html", "build/web/" GAME_NAME ".html" ) ) {
        cb_error( "Failed to rename " GAME_NAME ".html to index.html!" );
        return 1;
    }

    return 0;
}

int build_dependency_raylib(
    enum Target target, const char* cc, const char* ar
) {
    const char* deps[] = {
        "rcore",
        "rshapes",
        "rtextures",
        "rtext",
        "utils",
        "rmodels",
        "raudio",
        "rglfw",
    };
    PID pid[static_array_len(deps)];
    int count = static_array_len(deps);
    if( target == T_WEB ) {
        count -= 1;
    }

    char src_buf[255] = {};
    char dst_buf[255] = {};

    Command cmd;
    switch( target ) {
        case T_GNU_LINUX: {
            cmd = command_new( cc, src_buf, "-o", dst_buf, GNU_LINUX_RAYLIB_ARGS );
        } break;
        case T_WINDOWS: {
            cmd = command_new( cc, src_buf, "-o", dst_buf, WINDOWS_RAYLIB_ARGS );
        } break;
        case T_WEB: {
            cmd = command_new( cc, src_buf, "-o", dst_buf, WEB_RAYLIB_ARGS );
        } break;

        case T_NATIVE:
        case T_COUNT: unreachable();
    }

    const char* target_name = target_to_string(target).cc;

    CommandBuilder ar_builder;
    command_builder_new( ar, &ar_builder );
    command_builder_push( &ar_builder, "rcs" );
    // NOTE(alicia): lifetime of local buffer doesn't matter
    // because strings are copied into a seperate buffer.
    command_builder_push( &ar_builder, local_fmt( "vendor/%s/libraylib.a", target_name ) );

    for( int i = 0; i < count; ++i ) {
        /* const char* src = local_fmt( "raylib/src/%s.c", deps[i] ); */
        /* const char* dst = local_fmt( "vendor/%s/%s.o", target_name, deps[i] ); */
        snprintf( src_buf, static_array_len(src_buf), "raylib/src/%s.c", deps[i] );
        snprintf( dst_buf, static_array_len(dst_buf), "vendor/%s/%s.o", target_name, deps[i] );
        command_builder_push( &ar_builder, dst_buf );

        if( path_exists( dst_buf ) ) {
            file_remove( dst_buf );
        }

        /* cmd.args[1] = src; */
        /* cmd.args[3] = dst; */

        pid[i] = process_exec( cmd, false, 0, 0, 0, 0 );

        memset( src_buf, 0, static_array_len(src_buf) );
        memset( dst_buf, 0, static_array_len(dst_buf) );
    }

    for( int i = 0; i < count; ++i ) {
        int res = process_wait( pid[i] );
        if( res ) {
            for( int j = i; j < count; ++j ) {
                process_discard( pid[j] );
            }
            command_builder_free( &ar_builder );
            cb_error( "Failed to compile %s!", deps[i] );
            return res;
        }
    }

    cmd         = command_builder_cmd( &ar_builder );
    PID ar_proc = process_exec( cmd, false, 0, 0, 0, 0 );
    int res     = process_wait( ar_proc );

    command_builder_free( &ar_builder );
    if( res ) {
        cb_error( "Failed to create archive!" );
    }

    return res;
}

int __quick_cmd( Command _cmd ) {
    PID pid = process_exec( _cmd, false, 0, 0, 0, 0 );
    return process_wait( pid );
}
bool __make_dirs( const char* first, ... ) {
    va_list va;
    va_start( va, first );

    const char* current = first;
    for( ;; ) {
        if( !path_exists( current ) ) {
            cb_info( "Creating directory %s . . .", current );
            if( !dir_create( current ) ) {
                cb_error( "Failed to create directory %s!", current );
                va_end( va );
                return false;
            }
        }

        current = va_arg( va, const char* );
        if( !current ) {
            break;
        }
    }

    va_end( va );
    return true;
}

int mode_help( struct Args* args ) {
    enum Mode mode;
    if( args->mode == M_HELP ) {
        mode = args->help.mode;
    } else {
        mode = args->mode;
    }

    printf( "OVERVIEW:    Compile BIGMODE 2025 game jam submission.\n" );
    printf( "USAGE:       ./cbuild %s [args]\n",
        mode == M_HELP ? "<mode>" : mode_to_string(mode).cc );
    printf( "DESCRIPTION:\n");
    printf( "    %s\n", mode_description(mode).cc );
    printf( "ARGUMENTS:\n" );

    switch( mode ) {
        case M_HELP: {
            printf( "  <mode>  Print help for mode.\n" );
            printf( "            valid: ");
            for( enum Mode m = 0; m < M_COUNT; ++m ) {
                printf( "%s", mode_to_string(m).cc );
                if( m + 1 < M_COUNT ) {
                    printf( ", " );
                }
            }
            printf("\n");
        } break;
        case M_BUILD: {
            printf( "  -target=<target>  Set output target.\n" );
            printf( "                      valid: " );
            for( enum Target t = 0; t < T_COUNT; ++t ) {
                printf( "%s", target_to_string(t).cc );
                if( t + 1 < T_COUNT ) {
                    printf( ", " );
                }
            }
            printf("\n");
            printf( "  -optimized        Optimize with -O2 rather than -O0\n" );
            printf( "  -strip-symbols    Strip debug symbols.\n" );
        } break;
        case M_RUN: {
            printf( "  -optimized        Optimize with -O2 rather than -O0\n" );
            printf( "  -strip-symbols    Strip debug symbols.\n" );
        } break;
        case M_PACKAGE: {
        } break;
        case M_COUNT:
            break;
    }
    return 0;
}

String mode_description( enum Mode mode ) {
    switch( mode ) {
        case M_HELP:    return string_text("Print help for given mode.");
        case M_BUILD:   return string_text("Compile project.");
        case M_RUN:     return string_text("Compile and run (native only).");
        case M_PACKAGE: return string_text("Compile in Release mode and package for each platform (Windows,Linux and Web)");
        case M_COUNT: unreachable();
    }
}
String mode_to_string( enum Mode mode ) {
    switch( mode ) {
        case M_HELP:    return string_text("help");
        case M_BUILD:   return string_text("build");
        case M_RUN:     return string_text("run");
        case M_PACKAGE: return string_text("package");
        case M_COUNT: unreachable();
    }
}
bool mode_parse( String src, enum Mode* out_mode ) {
    for( enum Mode m = M_HELP; m < M_COUNT; ++m ) {
        if( string_cmp( mode_to_string(m), src ) ) {
            *out_mode = m;
            return true;
        }
    }
    return false;
}
String target_to_string( enum Target target ) {
    switch( target ) {
        case T_NATIVE:    return string_text( "native" );
        case T_GNU_LINUX: return string_text( "linux" );
        case T_WINDOWS:   return string_text( "windows" );
        case T_WEB:       return string_text( "web" );
        case T_COUNT: unreachable();
    }
}
String target_compiler( enum Target target ) {
    switch( target ) {
        case T_NATIVE:    return target_compiler( target_native() );
        case T_GNU_LINUX: {
#if defined(PLATFORM_WINDOWS)
            panic("Cannot cross-compile from windows to linux!");
#else
            return string_text("gcc");
#endif
        } break;
        case T_WINDOWS: {
#if defined(PLATFORM_WINDOWS)
            return string_text("gcc");
#else
            return string_text("x86_64-w64-mingw32-gcc-win32");
#endif
        } break;
        case T_WEB: {
            return string_text("emcc");
        } break;
        case T_COUNT: unreachable();
    }
}
String target_cpp_compiler( enum Target target ) {
    switch( target ) {
        case T_NATIVE:    return target_cpp_compiler( target_native() );
        case T_GNU_LINUX: {
#if defined(PLATFORM_WINDOWS)
            panic("Cannot cross-compile from windows to linux!");
#else
            return string_text("g++");
#endif
        } break;
        case T_WINDOWS: {
#if defined(PLATFORM_WINDOWS)
            return string_text("g++");
#else
            return string_text("x86_64-w64-mingw32-g++-win32");
#endif
        } break;
        case T_WEB: {
            return string_text("em++");
        } break;
        case T_COUNT: unreachable();
    }
}
String target_ar( enum Target target ) {
    switch( target ) {
        case T_NATIVE:    return target_cpp_compiler( target_native() );
        case T_GNU_LINUX: {
#if defined(PLATFORM_WINDOWS)
            panic("Cannot cross-compile from windows to linux!");
#else
            return string_text("ar");
#endif
        } break;
        case T_WINDOWS: {
#if defined(PLATFORM_WINDOWS)
            return string_text("ar");
#else
            return string_text("x86_64-w64-mingw32-gcc-ar-win32");
#endif
        } break;
        case T_WEB: {
            return string_text("emar");
        } break;
        case T_COUNT: unreachable();
    }
}
bool target_parse( String src, enum Target* out_target ) {
    for( enum Target t = T_NATIVE; t < T_COUNT; ++t ) {
        if( string_cmp( target_to_string(t), src ) ) {
            *out_target = t;
            return true;
        }
    }
    return false;
}
enum Target target_native(void) {
#if defined(PLATFORM_WINDOWS)
    return T_WINDOWS;
#else
    return T_GNU_LINUX;
#endif
}
bool target_is_native( enum Target target ) {
    switch( target ) {
        case T_NATIVE:    return target_is_native( target_native() );
        case T_GNU_LINUX: {
#if defined(PLATFORM_LINUX)
            return true;
#else
            return false;
#endif
        } break;
        case T_WINDOWS: {
#if defined(PLATFORM_WINDOWS)
            return true;
#else
            return false;
#endif
        } break;

        case T_WEB:
        case T_COUNT:     return false;
    }
}

#if 0 

#define stringify(macro) #macro
#define stringify_value(macro) stringify(macro)

#define GAME_NAME "bigmode2025"
// 1 gib
#define WEB_MAX_MEMORY 536870912

#if defined(PLATFORM_WINDOWS)
    #define BUILD_PATH_NATIVE "./build/native/" GAME_NAME ".exe"
#else
    #define BUILD_PATH_NATIVE "./build/native/" GAME_NAME
#endif

#define BUILD_PATH_WEB "./build/web/index.html"

enum Target {
    T_NATIVE,
    T_WINDOWS,
    T_GNU_LINUX,
    T_WEB,
};

int compile_raylib( enum Target target );
void print_help(void);
b32 dir_create_checked( const char* path );
int main( int argc, const char** argv ) {
    init( LOGGER_LEVEL_INFO );

    f64 start_time = timer_milliseconds();

    enum Target target = T_NATIVE;
    b32 test           = false;
    b32 release        = false;
    b32 package        = false;

    for( int i = 1; i < argc; ++i ) {
        String arg = string_from_cstr( argv[i] );

        if( string_cmp( arg, string_text( "--help" ) ) ) {
            print_help();
            return 0;
        }

        if( string_cmp(
            string_truncate( arg, sizeof("--target")),
            string_text("--target=")
        )) {
            String target_string = string_advance_by( arg, sizeof("--target"));

            if( string_cmp(
                target_string, string_text("native")
            ) ) {
                target = T_NATIVE;
                continue;
            } else if( string_cmp(
                target_string, string_text( "web" )
            ) ) {
                target = T_WEB;
                continue;
            }

        }

        if( string_cmp( arg, string_text( "--test" ) ) ) {
            test = true;
            continue;
        }

        if( string_cmp( arg, string_text( "--release" ) ) ) {
            release = true;
            continue;
        }

        if( string_cmp( arg, string_text( "--package" ) ) ) {
            package = true;
            continue;
        }

        cb_error( "unrecognized argument '%.*s'!", arg.len, arg.cc );
        print_help();
        return -1;
    }

    if( package ) {
        if( test ) {
            cb_warn(
                "--package and --test cannot be combined! "
                "--package overrides --test" );
        }
        release = true;
        test    = false;

        if( path_exists( "./build" ) ) {
            dir_remove( "./build", true );
        }
    }

    if( !dir_create_checked( "./vendor" ) ) {
        return -1;
    }
    if( !dir_create_checked( "./build") ) {
        return -1;
    }

    Command cmd = command_null();

    if( !process_in_path( "clang" ) ) {
        cb_error( "clang is required in path!" );
        return -1;
    }

    const char* build_path = "";
    switch( target ) {
        case T_NATIVE: {
            if( !dir_create_checked( "./build/native" ) ) {
                return -1;
            }
            if( !dir_create_checked( "./vendor/native" ) ) {
                return -1;
            }

            if( !path_exists( "./vendor/native/libraylib.a" ) ) {
                int res = compile_raylib( target );
                if( res ) {
                    return res;
                }
                cb_info( "compiled raylib for native platform!" );
            }

            build_path = BUILD_PATH_NATIVE;
            if( release ) {
                cmd = command_new(
                    "clang", "src/sources.c",
                    "vendor/native/libraylib.a",
                    "-Isrc", "-Iraylib/src",
                    "-static-libgcc", "-O2",
                    #if defined(PLATFORM_WINDOWS)
                        "-lraylib", "-lgdi32", "-lwinmm", "-lopengl32",
                        "-fuse-ld=lld", "-Wl,--subsystem,windows",
                    #elif defined(PLATFORM_LINUX)
                        "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",
                    #endif
                    "-Werror", "-Wall", "-Wextra", "-Werror=vla", "-o",
                    build_path );
            } else {
                cmd = command_new(
                    "clang", "src/sources.c",
                    "vendor/native/libraylib.a",
                    "-Isrc", "-Iraylib/src", "-DDEBUG",
                    "-static-libgcc", "-O0", "-g",
                    #if defined(PLATFORM_WINDOWS)
                        "-fuse-ld=lld", "-Wl,/debug", "-gcodeview",
                        "-lraylib", "-lgdi32", "-lwinmm", "-lopengl32",
                    #elif defined(PLATFORM_LINUX)
                        "-lGL", "-lm", "-lpthread", "-ldl", "-lrt", "-lX11",
                    #endif
                    "-Wall", "-Wextra", "-Werror=vla", "-o",
                    build_path );
            }

        } break;
        case T_WEB: {
            if( !dir_create_checked( "./build/web" ) ) {
                return -1;
            }
            if( !dir_create_checked( "./vendor/web" ) ) {
                return -1;
            }

            if( !process_in_path( "emcc" ) ) {
                cb_error( "emscripten is required in path!" );
                return -1;
            }

            if( !path_exists( "./vendor/web/libraylib.a" ) ) {
                int res = compile_raylib( target );
                if( res ) {
                    return res;
                }
                cb_info( "compiled raylib for web platform!" );
            }

            build_path = BUILD_PATH_WEB;
            const char* max_mem = "TOTAL_MEMORY=" stringify_value(WEB_MAX_MEMORY);

            if(release) {
                cmd = command_new(
                    #if defined(PLATFORM_WINDOWS)
                        "cmd", "/C", "emcc.bat",
                    #else
                        "sh", "-c", "emcc.sh",
                    #endif
                    "-o", build_path,
                    "src/sources.c",
                    "vendor/web/libraylib.a",
                    "-Os", "-Wall", "-Wextra", "-Werror=vla",
                    "-Werror", "-Isrc", "-Iraylib/src",
                    "-s", "USE_GLFW=3",
                    "-s", max_mem,
                    "--shell-file", "raylib/src/minshell.html",
                    "-DPLATFORM_WEB",
                    "--preload-file", "resources" );
            } else {
                cmd = command_new(
                    #if defined(PLATFORM_WINDOWS)
                        "cmd", "/C", "emcc.bat",
                    #else
                        "sh", "-c", "emcc.sh",
                    #endif
                    "-o", build_path,
                    "src/sources.c",
                    "vendor/web/libraylib.a",
                    "-g", "-O0", "-Wall", "-Wextra", "-Werror=vla",
                    "-Isrc", "-Iraylib/src", "-DDEBUG",
                    "-s", "USE_GLFW=3",
                    "-s", max_mem,
                    "--shell-file", "raylib/src/minshell.html",
                    "-DPLATFORM_WEB",
                    "--preload-file", "resources" );
            }
        } break;
    }

    if( release ) {
        cb_info( "building project in release mode . . ." );
    } else {
        cb_info( "building project in debug mode . . ." );
    }
    PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
    int res = process_wait( pid );

    if( !res ) {
        f64 end_time = timer_milliseconds();
        cb_info( "build completed in %fms", end_time - start_time );

        cb_info( "built project successfully at path '%s'!", build_path );
    } else {
        cb_error( "failed to build '%s'!", build_path );
        return res;
    }

    if( test ) {
        switch( target ) {
            case T_NATIVE: {
                cb_info( "testing project . . ." );
                Command test_cmd = command_new( build_path );
                PID test_pid = process_exec( test_cmd, false, NULL, NULL, NULL, NULL );
                int test_res = process_wait( test_pid );
                cb_info( "project exited with code %i", test_res );
            } break;
            case T_WEB: {
                cb_warn( "cannot automatically test web project!" );
            } break;
        }
    }

    if( package ) {
        if( !process_in_path( "zip" ) ) {
            cb_error( "zip is required for packaging project!" );
            return -1;
        }

        Command cmd = command_new( "zip", GAME_NAME, "resources", "-r" );
        PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
        int res = process_wait( pid );
        if( res ) {
            cb_error( "failed to zip project!" );
            return -1;
        }

        switch( target ) {
            case T_NATIVE: {
                if( !file_move(
                    "./build/native/" GAME_NAME ".zip",
                    GAME_NAME ".zip"
                ) ) {
                    cb_error( "failed to move zipped resources to build directory!" );
                    return -1;
                }

                const char* build_path =
                    GAME_NAME
                    #if defined(PLATFORM_WINDOWS)
                        ".exe"
                    #endif
                    ;
                cmd = command_new(
                    "zip", GAME_NAME ".zip",
                    build_path );

                pid = process_exec( cmd, false, NULL, NULL, NULL, "./build/native" );
                res = process_wait( pid );
                if( res ) {
                    cb_error( "failed to zip project!" );
                    return -1;
                }
                cb_info( "zipped project at path ./build/native/" GAME_NAME ".zip!" );
            } break;
            case T_WEB: {
                if( !file_move(
                    "./build/web/" GAME_NAME ".zip",
                    GAME_NAME ".zip"
                ) ) {
                    cb_error( "failed to move zipped resources to build directory!" );
                    return -1;
                }

                cmd = command_new( "zip", GAME_NAME ".zip", "*" );
                pid = process_exec( cmd, false, NULL, NULL, NULL, "./build/web" );
                res = process_wait( pid );
                if( res ) {
                    cb_error( "failed to zip project!" );
                    return -1;
                }
                cb_info( "zipped project at path ./build/web/" GAME_NAME ".zip!" );
            } break;
        }
    }

    return 0;
}

int compile_raylib( enum Target target ) {
    switch( target ) {
        case T_NATIVE: {
            cb_info( "compiling raylib for native platform . . ." );
            Command cmd =
                command_new(
                    "make", "-C", "./raylib/src", "-B",
                    "PLATFORM=PLATFORM_DESKTOP",
                    "CC=clang", "RAYLIB_RELEASE_PATH=../../vendor/native" );
            PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
            int res = process_wait( pid );
            if( res ) {
                cb_error( "failed to compile raylib for native platform!" );
                return res;
            }
        } break;
        case T_WEB: {
            cb_info( "compiling raylib for web . . ." );
            Command cmd =
                command_new(
                    "make", "-C", "./raylib/src", "-B",
                    "PLATFORM=PLATFORM_WEB",
                    "RAYLIB_RELEASE_PATH=../../vendor/web" );
            PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
            int res = process_wait( pid );
            if( res ) {
                cb_error( "failed to compile raylib for web platform!" );
                return res;
            }
        } break;
    }
    Command cmd = command_new(
        "make", "-C", "./raylib/src", "clean", "PLATFORM_SHELL=sh" );
    PID pid = process_exec( cmd, false, NULL, NULL, NULL, NULL );
    int res = process_wait( pid );
    if( res ) {
        cb_warn( "failed to remove raylib sources!" );
    }
    return 0;
}

#endif

#define CBUILD_IMPLEMENTATION
#include "cbuild.h"

