#if !defined(CBUILD_HEADER)
#define CBUILD_HEADER
/**
 * @file   cbuild.h
 * @brief  C Build System.
 * @details
 * Single header library for writing a build system in C.
 * Include to get API, include again (ONCE) with CBUILD_IMPLEMENTATION
 * defined to define implementation.
 *
 * Options can be defined before first include.
 *
 * Options:
 * - define CBUILD_THREAD_COUNT with number between 2 and 16 to set how many
 *   threads cbuild is allowed to initialize.
 * - define CBUILD_ASSERTIONS to get debug assertions in implementation and
 *   access to assertion() macro. The assertion() macro is otherwise defined
 *   to be unused( ... )
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   May 28, 2024
 * @copyright MIT License.
*/
// IWYU pragma: begin_exports
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
// IWYU pragma: end_exports

#if !(defined(COMPILER_CLANG) || defined(COMPILER_GCC) || defined(COMPILER_MSVC) || defined(COMPILER_UNKNOWN))
    #if defined(__clang__)
        /// @brief Current compiler is clang (LLVM).
        #define COMPILER_CLANG
    #elif defined(__GNUC__)
        /// @brief Current compiler is GCC.
        #define COMPILER_GCC
    #elif defined(_MSC_VER)
        /// @brief Current compiler is Microsoft Visual C++.
        #define COMPILER_MSVC
    #else
        /// @brief Current compiler is unknown.
        #define COMPILER_UNKNOWN
    #endif
#endif /* If no compiler is defined */

#if defined(_WIN32)
    /// @brief Current platform is Windows.
    #define PLATFORM_WINDOWS 1
    #if defined(__MINGW32__) || defined(__MINGW64__)
        /// @brief C Build compiled using MinGW subsystem.
        #define PLATFORM_MINGW 1
    #endif
#elif defined(__linux__)
    /// @brief Current platform is GNU/Linux.
    #define PLATFORM_LINUX 1
    /// @brief Current platform is POSIX compliant.
    #define PLATFORM_POSIX 1
#elif defined(__APPLE__)
    /// @brief Current platform is MacOS.
    #define PLATFORM_MACOS 1
    /// @brief Current platform is POSIX compliant.
    #define PLATFORM_POSIX 1
#else
    /// @brief Current platform is unknown.
    #define PLATFORM_UNKNOWN 1
    #if __has_include(<unistd.h>)
        /// @brief Current platform is POSIX compliant.
        #define PLATFORM_POSIX 1
    #endif
#endif

#if defined(__arm__) || defined(_M_ARM) || defined(__aarch64__) || defined(_M_ARM64)
    /// @brief Current CPU architecture is ARM-based.
    #define ARCH_ARM 1
#elif defined(__i386__) || defined(_M_IX86) || defined(__X86__) || defined(__x86_64__)
    /// @brief Current CPU architecture is x86 or x86-64.
    #define ARCH_X86 1
#else
    /// @brief Current CPU architecture is unknown.
    #define ARCH_UNKNOWN 1
#endif

#if UINT64_MAX == UINTPTR_MAX
    /// @brief Current CPU architecture is 64-bit.
    #define ARCH_64BIT 1
#else
    /// @brief Current CPU architecture is 32-bit.
    #define ARCH_32BIT 1
#endif

/// @brief Maximum path length expected.
#define CBUILD_PATH_CAPACITY (4096)

#if defined(PLATFORM_WINDOWS)
    #undef CBUILD_PATH_CAPACITY
    /// @brief Maximum path length expected.
    #define CBUILD_PATH_CAPACITY (8192) // this is to allow wide paths the same max length as narrow paths
#endif

/// @brief Number of local buffers per thread.
#define CBUILD_LOCAL_BUFFER_COUNT (4)
/// @brief Capacity of local buffers in bytes.
#define CBUILD_LOCAL_BUFFER_CAPACITY CBUILD_PATH_CAPACITY

/// @brief Maximum number of MT jobs.
#define CBUILD_MAX_JOBS (32)

/// @brief Minimum number of threads allowed.
#define CBUILD_THREAD_COUNT_MIN (1)
/// @brief Maximum number of threads allowed.
#define CBUILD_THREAD_COUNT_MAX (16)

#if !defined(CBUILD_THREAD_COUNT)
    /// @brief Number of threads to be spawned for job system.
    #define CBUILD_THREAD_COUNT (8)
#else
    #if CBUILD_THREAD_COUNT > CBUILD_THREAD_COUNT_MAX
        /// @brief Number of threads to be spawned for job system.
        #define CBUILD_THREAD_COUNT CBUILD_THREAD_COUNT_MAX
    #elif CBUILD_THREAD_COUNT < CBUILD_THREAD_COUNT_MIN
        /// @brief Number of threads to be spawned for job system.
        #define CBUILD_THREAD_COUNT CBUILD_THREAD_COUNT_MIN
    #endif
#endif

#if defined(COMPILER_MSVC)
    /// @brief Cross-compiler macro for declaring a thread local variable.
    #define make_tls( type )\
        __declspec( thread ) type
#else
    /// @brief Cross-compiler macro for declaring a thread local variable.
    #define make_tls( type )\
        _Thread_local type
#endif

#if defined(__cplusplus)
    #define restrict __restrict
#endif

/// @brief 8-bit unsigned.
typedef uint8_t   u8;
/// @brief 16-bit unsigned.
typedef uint16_t  u16;
/// @brief 32-bit unsigned.
typedef uint32_t  u32;
/// @brief 64-bit unsigned.
typedef uint64_t  u64;
/// @brief Pointer sized unsigned.
typedef uintptr_t usize;

/// @brief 8-bit signed.
typedef int8_t   i8;
/// @brief 16-bit signed.
typedef int16_t  i16;
/// @brief 32-bit signed.
typedef int32_t  i32;
/// @brief 64-bit signed.
typedef int64_t  i64;
/// @brief Pointer sized signed.
typedef intptr_t isize;

/// @brief Single-precision float.
typedef float  f32;
/// @brief Double-precision float.
typedef double f64;

/// @brief 8-bit Boolean, useful for struct packing.
/// @details When returned from library, guaranteed to be 0 or 1.
typedef u8  b8;
/// @brief 16-bit Boolean, useful for struct packing.
/// @details When returned from library, guaranteed to be 0 or 1.
typedef u16 b16;
/// @brief 32-bit Boolean, useful for struct packing.
/// @details When returned from library, guaranteed to be 0 or 1.
typedef u32 b32;
/// @brief 64-bit Boolean, useful for struct packing.
/// @details When returned from library, guaranteed to be 0 or 1.
typedef u64 b64;

#if defined(PLATFORM_WINDOWS)
    /// @brief Volatile 32-bit signed for atomic operations.
    typedef volatile long      atom;
    /// @brief Volatile 64-bit signed for atomic operations.
    typedef volatile long long atom64;
#else
    /// @brief Volatile 32-bit signed for atomic operations.
    typedef volatile i32 atom;
    /// @brief Volatile 64-bit signed for atomic operations.
    typedef volatile i64 atom64;
#endif

/// @brief String is dynamically allocated on the heap,
/// UTF-8 encoded and null-terminated.
typedef char DString;
/// @brief Macro for marking a dynamic array.
/// @details
/// Just pastes @c T *
/// @param T (type) Type of array.
#define Darray(T) T *
/// @brief String slice. Not necessarily null-terminated.
/// @note When printing, use "%.*s" format specifier and
/// provide length followed by pointer in args.
typedef struct {
    /// @brief ASCII length of string.
    usize len;
    /// @brief Pointer to start of string buffer.
    const char* cc;
} String;
/// @brief Logger levels.
typedef enum {
    /// @brief Info level.
    /// @details
    /// Most permissive, all logger messages allowed.
    LOGGER_LEVEL_INFO,
    /// @brief Warning level.
    /// @details
    /// Warning, Error and Fatal messages allowed.
    LOGGER_LEVEL_WARNING,
    /// @brief Error level.
    /// @details
    /// Error and Fatal messages allowed.
    LOGGER_LEVEL_ERROR,
    /// @brief Fatal level.
    /// @details
    /// Most restrictive level, only fatal messages allowed.
    LOGGER_LEVEL_FATAL,
} LoggerLevel;
/// @brief Types of file seek.
typedef enum {
    /// @brief Seek from current position.
    FSEEK_CURRENT,
    /// @brief Seek from start of file.
    FSEEK_BEGIN,
    /// @brief Seek from end of file.
    FSEEK_END,
} FileSeek;
/// @brief Flags for opening file.
typedef enum {
    /// @brief Open file for reading.
    FOPEN_READ     = (1 << 0),
    /// @brief Open file for writing.
    FOPEN_WRITE    = (1 << 1),
    /// @brief Create file. File must not exist if using this flag.
    FOPEN_CREATE   = (1 << 2),
    /// @brief Open and truncate file. File can only be opened for writing.
    FOPEN_TRUNCATE = (1 << 3),
    /// @brief Open and set position to end of file. File must be opened for writing.
    FOPEN_APPEND   = (1 << 4),
} FileOpenFlags;

#if defined(PLATFORM_WINDOWS)
    /// @brief Cross-platform file descriptor.
    typedef isize FD;
    /// @brief Cross-platform process ID.
    typedef isize PID;
#else
    /// @brief Cross-platform file descriptor.
    typedef int   FD;
    /// @brief Cross-platform process ID.
    typedef pid_t PID;
#endif
/// @brief Opaque mutex handle.
typedef struct {
    /// @brief Opaque handle.
    void* handle;
} Mutex;
/// @brief Opaque semaphore handle.
typedef struct {
    /// @brief Opaque handle.
    void* handle;
} Semaphore;
/// @brief Cross-platform pipe for reading.
typedef FD ReadPipe;
/// @brief Cross-platform pipe for writing.
typedef FD WritePipe;
/// @brief Cross-platform generic pipe (reading or writing).
typedef FD Pipe;
/// @brief Command line arguments for creating a process.
typedef struct {
    /// @brief Number of arguments.
    usize        count;
    /// @brief Array of arguments. Last item in array is a null-pointer.
    const char** args;
} Command;
/// @brief Command line argument builder.
typedef struct {
    /// @brief Character buffer.
    Darray(char) buf;
    /// @brief Array of strings.
    Darray(const char*) args;
} CommandBuilder;
/// @brief Result of walking a directory.
typedef struct {
    /// @brief Number of paths found in directory.
    usize    count;
    /// @brief Dynamic array of paths found in directory.
    Darray(String) paths;
    /// @brief String buffer.
    DString* buf;
} WalkDirectory;
/// @brief Hang thread on wait.
#define MT_WAIT_INFINITE (UINT32_MAX)
/// @brief Function prototype for job system.
/// @param[in] params (optional) Pointer to additional parameters.
typedef void JobFN( void* params );
/// @brief Filter prototype for string split functions.
/// @param     index  Index of the string being filtered.
/// @param     str    String to be filtered.
/// @param[in] params (optional) Pointer to additional parameters.
/// @return Filtered string.
typedef String StringSplitDelimFilterFN( usize index, String str, void* params );
/// @brief Filter prototype for dynamic array.
/// @param     index       Index of item being tested.
/// @param     item_stride Size of items in array.
/// @param[in] item        Pointer to item being tested.
/// @param[in] params      (optional) Pointer to additional parameters.
/// @return
///     - @c True  : Item passed filter, will be kept in.
///     - @c False : Item failed filter, will be filtered out.
typedef b32 DarrayFilterFN(
    usize index, usize item_stride, const void* item, void* params );

static inline void _0( int _, ... ) {(void)_;}
/// @brief Mark any variables/parameters as unused.
/// @param ... Variables to be marked as unused.
#define unused( ... ) _0( 0, __VA_ARGS__ )

/// @brief Insert a panic.
#define __insert_panic() exit(-1)

#if defined(COMPILER_MSVC)
    /// @brief Insert an unreachable statement.
    #define __insert_unreachable() __assume(0)
    /// @brief Insert a debugger break statement.
    #define insert_break()         __debugbreak()
    /// @brief Insert a runtime crash.
    #define __insert_crash()       __debugbreak()
    /// @brief Mark function as no return.
    #define does_not_return()      __declspec( noreturn )
#else
    /// @brief Insert an unreachable statement.
    #define __insert_unreachable() __builtin_unreachable()
    /// @brief Insert a debugger break statement.
    #define insert_break()         __builtin_debugtrap()
    /// @brief Insert a runtime crash.
    #define __insert_crash()       __builtin_trap()
    /// @brief Mark function as no return.
    #define does_not_return()      _Noreturn
#endif

/// @brief Stringify macro name.
/// @param macro (any defined) Macro to stringify
/// @return String literal.
#define macro_to_string( macro ) #macro
/// @brief Stringify macro value.
/// @param macro (any defined) Macro whose value will be stringified.
/// @return String literal.
#define macro_value_to_string( macro ) macro_to_string( macro )
/// @brief Calculate length of static array.
/// @param array (any[]) Static array to calculate length of.
/// @return (usize) Length of @c array.
#define static_array_len( array ) (sizeof(array) / sizeof((array)[0]))
/// @brief Convert kilobytes to bytes.
/// @param kb (size_t) Kilobytes.
/// @return (size_t) @c kb as bytes.
#define kilobytes( kb ) (kb * 1000ULL)
/// @brief Convert megabytes to bytes.
/// @param mb (size_t) Megabytes.
/// @return (size_t) @c mb as bytes.
#define megabytes( mb ) (kilobytes(mb) * 1000ULL)
/// @brief Convert gigabytes to bytes.
/// @param gb (size_t) Gigabytes.
/// @return (size_t) @c gb as bytes.
#define gigabytes( gb ) (megabytes(gb) * 1000ULL)
/// @brief Convert Terabytes to bytes.
/// @param tb (size_t) Terabytes.
/// @return (size_t) @c tb as bytes.
#define terabytes( tb ) (gigabytes(tb) * 1000ULL)
/// @brief Convert kibibytes to bytes.
/// @param kb (size_t) Kibibytes.
/// @return (size_t) @c kb as bytes.
#define kibibytes( kb ) (kb * 1024ULL)
/// @brief Convert mebibytes to bytes.
/// @param mb (size_t) Mebibytes.
/// @return (size_t) @c mb as bytes.
#define mebibytes( mb ) (kibibytes(mb) * 1024ULL)
/// @brief Convert gibibytes to bytes.
/// @param gb (size_t) Gibibytes.
/// @return (size_t) @c gb as bytes.
#define gibibytes( gb ) (mebibytes(gb) * 1024ULL)
/// @brief Convert tebibytes to bytes.
/// @param tb (size_t) Tebibytes.
/// @return (size_t) @c tb as bytes.
#define tebibytes( tb ) (gibibytes(tb) * 1024ULL)

/// @brief Function for initializing cbuild. Must be called from main()
/// @param logger_level Level to set logger to.
#define init( logger_level )\
    _init_( logger_level, argv[0], __FILE__, argc, (const char**)argv )
/// @brief Rebuild cbuild.
/// @param[in] cbuild_source_file_name Filename of source file (__FILE__).
/// @param[in] cbuild_executable_name  Name of cbuild executable (argv[0]).
/// @param     reload                  Reloads cbuild after rebuilding. Only supported on POSIX platforms.
does_not_return() void cbuild_rebuild(
    const char* cbuild_source_file_name,
    const char* cbuild_executable_name, b32 reload );

/// @brief Allocate memory on the heap. Always returns zeroed memory.
/// @warning
/// Do not free with free(), use memory_free() instead!
/// Do not reallocate with realloc(), use memory_realloc() instead!
/// @param size Size in bytes of memory to allocate.
/// @return
///     - Pointer : Allocation succeeded, pointer to start of buffer.
///     - NULL    : Allocation failed.
void* memory_alloc( usize size );
/// @brief Reallocate memory on the heap. Always returns zeroed memory.
/// @warning
/// Do not free with free(), use memory_free() instead!
/// Do not reallocate with realloc(), use memory_realloc() instead!
/// @param[in] memory   Pointer to block of memory to reallocate.
/// @param     old_size Size in bytes of memory to reallocate.
/// @param     new_size New size of memory to reallocate. Must be > @c old_size.
/// @return
///     - Pointer : Allocation succeeded, pointer to start of buffer.
///     - NULL    : Allocation failed.
void* memory_realloc( void* memory, usize old_size, usize new_size );
/// @brief Free memory allocated on the heap.
/// @warning
/// Only free pointers from memory_alloc() or memory_realloc()!
/// Do not use with pointer from malloc()!
/// @param[in] memory Pointer to block of memory to free.
/// @param     size   Size in bytes of block of memory.
void  memory_free( void* memory, usize size );
/// @brief Query how many bytes are currently allocated.
/// @return Bytes allocated.
usize memory_query_usage(void);
/// @brief Query how many bytes have been allocated thus far.
/// @return Bytes allocated in total.
usize memory_query_total_usage(void);
/// @brief Copy value across block of memory.
/// @param[in] memory     Block to modify, must be >= @c value_size in size.
/// @param     value_size Size of value to stamp.
/// @param[in] value      Pointer to value to stamp.
/// @param     size       Total size of @c memory in bytes.
void memory_stamp( void* memory, usize value_size, const void* value, usize size );
/// @brief Set bytes in block of memory to a value.
/// @param[in] memory Block to modify.
/// @param     value  Value to set bytes to.
/// @param     size   Size of @c memory in bytes.
void memory_set( void* memory, i8 value, usize size );
/// @brief Set bytes in block of memory to zero.
/// @param[in] memory Block to modify.
/// @param     size   Size of @c memory in bytes.
void memory_zero( void* memory, usize size );
/// @brief Copy bytes from one block of memory to another.
/// @details
/// @c dst and @c src cannot be overlapping.
/// Use memory_move() instead for overlapping memory.
/// @param[in] dst  Pointer to destination.
/// @param[in] src  Pointer to source.
/// @param     size Size of @c dst and @c src.
void memory_copy( void* restrict dst, const void* restrict src, usize size );
/// @brief Copy bytes from one block of memory to another.
/// @details
/// @c dst and @c src can be overlapping.
/// @param[in] dst  Pointer to destination.
/// @param[in] src  Pointer to source.
/// @param     size Size of @c dst and @c src.
void memory_move( void* dst, const void* src, usize size );
/// @brief Compare bytes from two blocks of memory for equality.
/// @param[in] a, b Pointer to blocks of memory to compare.
/// @param     size Size of @c dst and @c src.
/// @return
///     - @c True  : @c a and @c b are equal.
///     - @c False : @c a and @c b are not equal.
b32 memory_cmp( const void* a, const void* b, usize size );

/// @brief Check if character is in set.
/// @param c   Character to check for.
/// @param set Set of characters to check against.
/// @return
///     - @c True  : @c c is in @c set.
///     - @c False : @c c is not in @c set.
b32 char_in_set( char c, String set );

/// @brief Calculate UTF-8 length of null-terminated string.
/// @note
/// Does not check if @c string is a valid UTF-8 string!
/// @param[in] string Pointer to character buffer.
/// @return UTF-8 length of @c string.
usize strlen_utf8( const char* string );

/// @brief Create an empty string.
/// @return Empty @c string.
#define string_empty()\
    (String){ .len=0, .cc=0 }
/// @brief Create a new string from length and buffer.
/// @param     length (usize) Length of string buffer.
/// @param[in] buf    (const char*) Pointer to start of buffer.
/// @return New @c string.
#define string_new( length, buf )\
    (String){ .len=length, .cc=buf }
/// @brief Create a new string from string literal.
/// @param literal (string literal) Literal to make string out of.
/// @return New @c string.
#define string_text( literal )\
    string_new( sizeof(literal) - 1, literal )
/// @brief Create a new string from null-terminated string.
/// @param s (const char*) Null-terminated to make string out of.
/// @return New @c string.
#define string_from_cstr( s )\
    string_new( strlen( s ), s )
/// @brief Create a new string from dynamic string.
/// @param d (const DString*) Dynamic string to make string out of.
/// @return New @c string.
#define string_from_dstring( d )\
    string_new( dstring_len( d ), d )
/// @brief Check if string is empty.
/// @param str String to check.
/// @return
///     - @c True  : @c str is empty.
///     - @c False : @c str is not empty.
b32 string_is_empty( String str );
/// @brief Check if string is null-terminated.
/// @param str String to check.
/// @return
///     - @c True  : @c str is null-terminated.
///     - @c False : @c str is not null-terminated.
b32 string_is_null_terminated( String str );
/// @brief Compare two strings for equality.
/// First compares string lengths, then compares contents.
/// @param a, b Strings to compare.
/// @return
///     - @c True  : Strings @c a and @c b are equal.
///     - @c False : Strings @c a and @c b are not equal.
b32 string_cmp( String a, String b );
/// @brief Search for character in string.
/// @param      str           String to search in.
/// @param      c             ASCII character to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of @c c if it was found.
/// @return
///     - @c True  : @c c was found in @c string.
///     - @c False : @c c was not found in @c string.
b32 string_find( String str, char c, usize* opt_out_index );
/// @brief Search for character in string from end of string.
/// @param      str           String to search in.
/// @param      c             ASCII character to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of @c c if it was found.
/// @return
///     - @c True  : @c c was found in @c string.
///     - @c False : @c c was not found in @c string.
b32 string_find_rev( String str, char c, usize* opt_out_index );
/// @brief Search for a set of characters in string.
/// @param      str           String to search in.
/// @param      set           Set of ASCII characters to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of first character in set found.
/// @return
///     - @c True  : Character in @c set was found in @c string.
///     - @c False : Character in @c set was not found in @c string.
b32 string_find_set( String str, String set, usize* opt_out_index );
/// @brief Search for a set of characters in string from end of string.
/// @param      str           String to search in.
/// @param      set           Set of ASCII characters to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of first character in set found.
/// @return
///     - @c True  : Character in @c set was found in @c string.
///     - @c False : Character in @c set was not found in @c string.
b32 string_find_set_rev( String str, String set, usize* opt_out_index );
/// @brief Search for substring in string.
/// @param      str           String to search in.
/// @param      substr        String to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of @c substr if it was found.
/// @return
///     - @c True  : @c substr was found in @c string.
///     - @c False : @c substr was not found in @c string.
b32 string_find_string( String str, String substr, usize* opt_out_index );
/// @brief Search for substring in string from end of string.
/// @param      str           String to search in.
/// @param      substr        String to search for.
/// @param[out] opt_out_index (optional) Pointer to store index of @c substr if it was found.
/// @return
///     - @c True  : @c substr was found in @c string.
///     - @c False : @c substr was not found in @c string.
b32 string_find_string_rev( String str, String substr, usize* opt_out_index );
/// @brief Get pointer to first character in string.
/// @param str String to get character from.
/// @return
///     - @c Pointer : @c str is not empty.
///     - @c NULL    : @c str is empty.
char* string_first( String str );
/// @brief Get pointer to last character in string.
/// @param str String to get character from.
/// @return
///     - @c Pointer : @c str is not empty.
///     - @c NULL    : @c str is empty.
char* string_last( String str );
/// @brief Advance string forward by one character.
/// @param str String to advance.
/// @return
///     - @c string       : String one character forward.
///     - @c empty string : String has no more characters.
String string_advance( String str );
/// @brief Advance string forward by given number of characters.
/// @param str    String to advance.
/// @param stride Number of characters to advance by.
/// @return
///     - @c string       : String @c stride number of characters forward.
///     - @c empty string : String has no more characters or stride >= @c str.len.
String string_advance_by( String str, usize stride );
/// @brief Shorten string length to maximum.
/// @details
/// Does nothing if max > str.len.
/// Returns empty string if max == 0.
/// @param str String to truncate.
/// @param max Maximum length to truncate to.
/// @return Truncated string.
String string_truncate( String str, usize max );
/// @brief Subtract from string length.
/// @details
/// Does nothing if amount == 0. 
/// Returns empty string if amount >= str.len.
/// @param str    String to trim.
/// @param amount Number of characters to trim from the end.
/// @return Trimmed string.
String string_trim( String str, usize amount );
/// @brief Remove leading whitespace characters from string.
/// @param str String to remove from.
/// @return @c str without leading whitespace.
String string_trim_leading_whitespace( String str );
/// @brief Remove trailing whitespace characters from string.
/// @param str String to remove from.
/// @return @c str without trailing whitespace.
String string_trim_trailing_whitespace( String str );
/// @brief Remove leading and trailing whitespace characters from string.
/// @param str String to remove from.
/// @return @c str without leading or trailing whitespace.
String string_trim_surrounding_whitespace( String str );
/// @brief Split string in two from given index.
/// @warning
/// expect() 's that @c at is <= @c str.len!
/// @param      src           String to split.
/// @param      at            Index to split at.
/// @param      keep_split    True to keep index in right side, false to remove it.
/// @param[out] opt_out_left  (optional) Pointer to write left side to.
/// @param[out] opt_out_right (optional) Pointer to write right side to.
void string_split_at(
    String src, usize at, b32 keep_split,
    String* opt_out_left, String* opt_out_right );
/// @brief Split string at first instance of character.
/// @param      src           String to split.
/// @param      c             Character to split at.
/// @param      keep_split    True to keep character in right side, false to remove it.
/// @param[out] opt_out_left  (optional) Pointer to write left side to.
/// @param[out] opt_out_right (optional) Pointer to write right side to.
/// @return
///     - @c True  : Character @c c was found in @c src and string was split.
///     - @c False : Character @c c was not found in @c src.
b32 string_split_char(
    String src, char c, b32 keep_split,
    String* opt_out_left, String* opt_out_right );
/// @brief Split string by delimiter. Allocates dynamic array.
/// @param src        String to split.
/// @param delim      Delimiter to split by.
/// @param keep_delim True if delimiter should be kept in splits.
/// @return
///     - @c Dynamic array of strings : Dynamic array was allocated with splits.
///     - @c NULL : Failed to allocate dynamic array of splits.
String* string_split_delim( String src, String delim, b32 keep_delim );
/// @brief Split string by delimiter. Allocates dynamic array.
/// @param src        String to split.
/// @param delim      Delimiter character to split by.
/// @param keep_delim True if delimiter should be kept in splits.
/// @return
///     - @c Dynamic array of strings : Dynamic array was allocated with splits.
///     - @c NULL : Failed to allocate dynamic array of splits.
#define string_split_delim_char( src, delim, keep_delim )\
    string_split_delim( src, string_new( 1, (char[]){ delim } ), keep_delim )
/// @brief Split string by delimiter. Allocates dynamic array.
/// @param     src        String to split.
/// @param     delim      Delimiter to split by.
/// @param     keep_delim True if delimiter should be kept in splits.
/// @param[in] filter     Function to further process splits.
/// @param[in] params     (optional) Parameters for filtering function.
/// @return
///     - @c Dynamic array of strings : Dynamic array was allocated with splits.
///     - @c NULL : Failed to allocate dynamic array of splits.
String* string_split_delim_ex(
    String src, String delim, b32 keep_delim,
    StringSplitDelimFilterFN* filter, void* params );
/// @brief Split string by delimiter. Allocates dynamic array.
/// @param     src        String to split.
/// @param     delim      Delimiter character to split by.
/// @param     keep_delim True if delimiter should be kept in splits.
/// @param[in] filter     Function to further process splits.
/// @param[in] params     (optional) Parameters for filtering function.
/// @return
///     - @c Dynamic array of strings : Dynamic array was allocated with splits.
///     - @c NULL : Failed to allocate dynamic array of splits.
#define string_split_delim_char_ex( src, delim, keep_delim, filter, params )\
    string_split_delim_ex( src, string_new( 1, (char[]){ delim } ),\
        keep_delim, filter, params )
/// @brief Calculate UTF-8 length of string.
/// @note
/// Does not check if @c string is a valid UTF-8 string!
/// @param str String to get UTF-8 length of.
/// @return UTF-8 length of @c str.
usize string_len_utf8( String str );

/// @brief Allocate an empty dynamic string with given capacity.
/// @param cap Capacity to allocate.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_empty( usize cap );
/// @brief Create new dynamic string from string buffer.
/// @details
/// Allocates appropriately sized dynamic string and
/// copies characters from @c str into it.
/// Dynamic strings are always null-terminated.
/// @param     len Length of string buffer.
/// @param[in] str Pointer to start of string buffer.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_new( usize len, const char* str );
/// @brief Create a new dynamic string from null-terminated string.
/// @details
/// Allocates appropriately sized dynamic string and
/// copies characters from @c str into it.
/// Dynamic strings are always null-terminated.
/// @param[in] str (const char*) Pointer to null-terminated string.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
#define dstring_from_cstr( str ) dstring_new( strlen( str ), str )
/// @brief Create a new dynamic string from string.
/// @details
/// Allocates appropriately sized dynamic string and
/// copies characters from @c str into it.
/// Dynamic strings are always null-terminated.
/// @param str (string) String.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
#define dstring_from_string( str ) dstring_new( str.len, str.cc )
/// @brief Create a new dynamic string from string.
/// @details
/// Allocates appropriately sized dynamic string and
/// copies characters from @c str into it.
/// This function only exists so that string pointers
/// do not have to be dereferenced.
/// Dynamic strings are always null-terminated.
/// @param[in] str (string) Pointer to string.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
#define dstring_from_string_ptr( str ) dstring_new( (str)->len, (str)->cc )
/// @brief Create a new dynamic string from string literal.
/// @details
/// Allocates appropriately sized dynamic string and
/// copies characters from @c literal into it.
/// @param literal (string literal) Literal.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
#define dstring_text( literal ) dstring_new( sizeof(literal)-1, literal )
/// @brief Create string from dynamic string.
/// @param dstr (DString*) Dynamic string to create string from.
/// @return (string) String.
#define dstring_to_string( dstr ) string_from_dstring( dstr )
/// @brief Create a formatted dynamic string.
/// @param[in] format Format string literal.
/// @param     va     Variadic list of formatting arguments.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_fmt_va( const char* format, va_list va );
/// @brief Create a formatted dynamic string.
/// @param[in] format Format string literal.
/// @param     ...    Format arguments.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_fmt( const char* format, ... );
/// @brief Attempt to reallocate dynamic string buffer.
/// @param[in] str    Dynamic string to reallocate.
/// @param     amount Additional capacity to reallocate. Gets added to existing capacity.
/// @return 
///     - @c Dynamic String : Buffer was successfully reallocated.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_grow( DString* str, usize amount );
/// @brief Create a new dynamic string from existing dynamic string.
/// @param[in] src Dynamic string to clone.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_clone( const DString* src );
/// @brief Create dynamic string by concatenating two strings.
/// @param lhs, rhs Strings to concatenate.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_concat( String lhs, String rhs );
/// @brief Create dynamic string by concatenating two null-terminated strings.
/// @param[in] lhs, rhs (const char*) Strings to concatenate.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
#define dstring_concat_cstr( lhs, rhs )\
    dstring_concat( string_from_cstr( lhs ), string_from_cstr( rhs ) )
/// @brief Create dynamic string by concatenating multiple strings.
/// @param     count         Number of strings to concatenate.
/// @param[in] strings       Pointer to strings to concatenate.
/// @param     opt_separator (optional) String to use to separate @c strings.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_concat_multi(
    usize count, const String* strings, String opt_separator );
/// @brief Create dynamic string by concatenating multiple null-terminated strings.
/// @param     count         Number of strings to concatenate.
/// @param[in] strings       Pointer to strings to concatenate.
/// @param[in] opt_separator (optional) String to use to separate @c strings.
/// @return
///     - @c Dynamic String : Buffer was successfully allocated.
///     - @c NULL : Failed to allocate dynamic string.
DString* dstring_concat_multi_cstr(
    usize count, const char** strings, const char* opt_separator );
/// @brief Append string to end of dynamic string.
/// @param[in] str    Dynamic string to append to.
/// @param     append String to append.
/// @return
///     - @c Dynamic String : Append was successful.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_append( DString* str, String append );
/// @brief Append null-terminated string to end of dynamic string.
/// @param[in] str    Dynamic string to append to.
/// @param[in] append Null-terminated string to append.
/// @return
///     - @c Dynamic String : Append was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_append_cstr( str, append )\
    dstring_append( str, string_from_cstr( append ) )
/// @brief Append string literal to end of dynamic string.
/// @param[in] str     (DString*)       Dynamic string to append to.
/// @param     literal (string litearl) String literal to append.
/// @return
///     - @c Dynamic String : Append was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_append_text( str, literal )\
    dstring_append( str, string_text( literal ) )
/// @brief Prepend string to start of dynamic string.
/// @param[in] str     Dynamic string to prepend to.
/// @param     prepend String to append.
/// @return
///     - @c Dynamic String : Prepend was successful.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_prepend( DString* str, String prepend );
/// @brief Prepend null-terminated string to end of dynamic string.
/// @param[in] str     Dynamic string to prepend to.
/// @param[in] prepend Null-terminated string to prepend.
/// @return
///     - @c Dynamic String : Prepend was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_prepend_cstr( str, prepend )\
    dstring_prepend( str, string_from_cstr( prepend ) )
/// @brief Prepend string literal to end of dynamic string.
/// @param[in] str     (DString*)       Dynamic string to prepend to.
/// @param     literal (string litearl) String literal to prepend.
/// @return
///     - @c Dynamic String : Prepend was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_prepend_text( str, literal )\
    dstring_prepend( str, string_text( literal ) )
/// @brief Insert string inside dynamic string.
/// @param[in] str    Dynamic string to insert in.
/// @param     insert String to insert.
/// @param     at     Index to insert at. Must be <= @c str.len.
/// @return
///     - @c Dynamic String : Insert was successful.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_insert( DString* str, String insert, usize at );
/// @brief Insert null-terminated string inside dynamic string.
/// @param[in] str    Dynamic string to insert in.
/// @param[in] insert String to insert.
/// @param     at     Index to insert at. Must be <= @c str.len.
/// @return
///     - @c Dynamic String : Insert was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_insert_cstr( str, insert, at )\
    dstring_insert( str, string_from_cstr( insert ), at )
/// @brief Insert string literal inside dynamic string.
/// @param[in] str     Dynamic string to insert in.
/// @param     literal (string literal) String to insert.
/// @param     at      Index to insert at. Must be <= @c str.len.
/// @return
///     - @c Dynamic String : Insert was successful.
///     - @c NULL : Failed to reallocate dynamic string.
#define dstring_insert_text( str, literal, at )\
    dstring_insert( str, string_text( literal ), at )
/// @brief Append character to end of dynamic string.
/// @param[in] str Dynamic string to push into.
/// @param     c   Character to push.
/// @return
///     - @c Dynamic String : Push was successful.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_push( DString* str, char c );
/// @brief Emplace character inside of dynamic string.
/// @param[in] str Dynamic string to emplace into.
/// @param     c   Character to emplace.
/// @param     at  Index to emplace at.
/// @return
///     - @c Dynamic String : Push was successful.
///     - @c NULL : Failed to reallocate dynamic string.
DString* dstring_emplace( DString* str, char c, usize at );
/// @brief Pop last character from dynamic string, if available.
/// @param[in]  str       Dynamic string to pop character from.
/// @param[out] opt_out_c (optional) Pointer to write popped character to.
/// @return
///     - @c True  : @c str has a character to pop.
///     - @c False : @c str is empty.
b32 dstring_pop( DString* str, char* opt_out_c );
/// @brief Remove character from dynamic string.
/// @param[in] str   Dynamic string to remove character from.
/// @param     index Index of character to remove.
/// @return
///     - @c True  : @c index was in bounds of @c str and was removed.
///     - @c False : @c index was out of bounds.
b32 dstring_remove( DString* str, usize index );
/// @brief Remove range of characters from dynamic string.
/// @details
/// With assertions on, asserts that from < to.
/// @param[in] str            Dynamic string to remove characters from.
/// @param     from_inclusive Start of range, inclusive.
/// @param     to_exclusive   End of range, exclusive. (can be == length).
/// @return
///     - @c True  : Range was in bounds of @c str and was removed.
///     - @c False : Range was out of bounds.
b32 dstring_remove_range( DString* str, usize from_inclusive, usize to_exclusive );
/// @brief Shorten dynamic string length to maximum.
/// @details
/// Does nothing if max > str.len.
/// Returns empty string if max == 0.
/// @param[in] str Dynamic string to truncate.
/// @param     max Maximum length to truncate to.
void dstring_truncate( DString* str, usize max );
/// @brief Subtract from dynamic string length.
/// @details
/// Does nothing if amount == 0. 
/// Returns empty string if amount >= str.len.
/// @param[in] str    Dynamic string to trim.
/// @param     amount Number of characters to trim from the end.
void dstring_trim( DString* str, usize amount );
/// @brief Set length of dynamic string to zero and zero out memory.
/// @note This does not free the string!
/// @param[in] str Dynamic string to clear.
void dstring_clear( DString* str );
/// @brief Calculate remaining capacity in dynamic string.
/// @param[in] str Dynamic string.
/// @return Remaining capacity. Does not include null byte.
usize dstring_remaining( const DString* str );
/// @brief Get length of dynamic string.
/// @param[in] str Dynamic string.
/// @return Length not including null byte.
usize dstring_len( const DString* str );
/// @brief Get capacity of dynamic string.
/// @param[in] str Dynamic string.
/// @return Total capacity, includes null byte.
usize dstring_cap( const DString* str );
/// @brief Get total heap memory usage of dynamic string.
/// @param[in] str Dynamic string.
/// @return Total memory usage.
usize dstring_total_size( const DString* str );
/// @brief Check if dynamic string is empty.
/// @param[in] str Dynamic string.
/// @return
///     - @c True  : Length is zero.
///     - @c False : Length is not zero.
b32 dstring_is_empty( const DString* str );
/// @brief Check if dynamic string is full.
/// @param[in] str Dynamic string.
/// @return
///     - @c True  : Length equals capacity - 1.
///     - @c False : Dynamic string has remaining capacity.
b32 dstring_is_full( const DString* str );
/// @brief Set length of dstring.
/// @details
/// Panics if @c len exceeds string capacity.
///
/// This function also sets null-terminator.
/// @param[in] str Dynamic string.
/// @param     len Length to set.
void dstring_set_len( DString* str, usize len );
/// @brief Get mutable pointer to start of dynamic string buffer.
/// @details
/// Used internally.
/// Returns NULL if @c str is NULL.
/// @param[in] str Dynamic string.
/// @return Start of dynamic string.
void* dstring_head( DString* str );
/// @brief Get pointer to start of dynamic string buffer.
/// @details
/// Used internally.
/// Returns NULL if @c str is NULL.
/// @param str Dynamic string.
/// @return Start of dynamic string.
const void* dstring_head_const( const DString* str );
/// @brief Free a dynamic string.
/// @details
/// Use this function instead of memory_free() as string pointer
/// is not the pointer allocated from system.
/// @param[in] str Dynamic string.
void dstring_free( DString* str );

/// @brief Allocate an empty dynamic array.
/// @param stride Size of items in dynamic array.
/// @param cap    Number of items dynamic array should be able to hold.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
void* darray_empty( usize stride, usize cap );
/// @brief Create dynamic array from existing array.
/// @param     stride Size of items in array.
/// @param     len    Length of array.
/// @param[in] buf    Pointer to start of array.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
void* darray_from_array( usize stride, usize len, const void* buf );
/// @brief Calculate memory requirement for dynamic array.
/// @details
/// Used to create dynamic array with fixed memory size.
/// @param stride Size of items in dynamic array.
/// @param cap    Number of items dynamic array should be able to hold.
/// @return Required size of dynamic array buffer.
usize darray_static_memory_requirement( usize stride, usize cap );
/// @brief Create a dynamic array from static memory. (non reallocating)
/// @note
/// Returned pointer is not the same as @c buf!
/// Do not free this array with darray_free(),
/// use result of darray_head() or @c buf to free.
/// @param     stride Size of items in dynamic array.
/// @param     cap    Number of items dynamic array can hold.
/// @param[in] buf    Start of buffer. Must be able to hold result from darray_static_memory_requirement().
/// @return Start of dynamic array buffer.
void* darray_static( usize stride, usize cap, void* buf );
/// @brief Create a dynamic array from item literals.
/// @param type (type) Type of items.
/// @param ...  (any of type @c type) Items to fill array with.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
#define darray_literal( type, ... )\
    darray_from_array( sizeof(type),\
        sizeof((type[]){ __VA_ARGS__ }) / sizeof(type), (type[]){ __VA_ARGS__ } )
/// @brief Concatenate two arrays into one darray.
/// @note
/// Arrays provided should hold the same type of data.
/// @param     stride  Size of items in @c lhs and @c rhs.
/// @param     lhs_len Length of @c lhs.
/// @param[in] lhs     Pointer to start of left hand array.
/// @param     rhs_len Length of @c rhs.
/// @param[in] rhs     Pointer to start of right hand array.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
void* darray_join( usize stride,
    usize lhs_len, const void* lhs, usize rhs_len, const void* rhs );
/// @brief Create dynamic array from filtered array.
/// @param     stride        Size of items in @c src.
/// @param     len           Length of @c src.
/// @param[in] src           Pointer to start of array to be filtered.
/// @param[in] filter        Pointer to filter function.
/// @param[in] filter_params (optional) Parameters to filter function.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
void* darray_from_filter(
    usize stride, usize len, const void* src,
    DarrayFilterFN* filter, void* filter_params );
/// @brief Grow dynamic array by @c amount number of items.
/// @param[in] darray Dynamic array to grow.
/// @param     amount Number of items to grow by.
/// @return
///     - @c Pointer : Reallocated dynamic array capable of holding cap + @c amount.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_grow( void* darray, usize amount );
/// @brief Create new dynamic array from existing dynamic array.
/// @param[in] darray Dynamic array to clone.
/// @return
///     - @c Pointer : Pointer to start of dynamic array.
///     - @c NULL    : Failed to allocate dynamic array.
void* darray_clone( const void* darray );
/// @brief Set dynamic array length to zero and zero out existing items.
/// @param[in] darray Dynamic array to clear.
void darray_clear( void* darray );
/// @brief Set length of dynamic array.
/// @details
/// Fallible:
/// - If @c len is > darray.cap, reallocates dynamic array.
///
/// Guaranteed to succeed:
/// - If @c len is > darray.len and < darray.cap, sets new length and zeroes new items.
/// - If @c len < darray.len, same as calling darray_truncate().
/// @param[in] darray Dynamic array.
/// @param     len    New length.
/// @return
///     - @c Pointer : Pointer to dynamic array.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_set_len( void* darray, usize len );
/// @brief Set maximum length of dynamic array.
/// @details
/// If @c max >= darray.len, returns without modifying array.
/// Otherwise, truncates length and zeroes out max to length.
/// @param[in] darray Dynamic array to modify.
/// @param     max    Maximum length of dynamic array.
void darray_truncate( void* darray, usize max );
/// @brief Subtract from dynamic array length.
/// @details
/// If @c amount is >= darray.len, same as clearing darray.
/// @param[in] darray Dynamic array to modify.
/// @param     amount Number of items to substract from length.
void darray_trim( void* darray, usize amount );
/// @brief Attempt to push new item to end of dynamic array.
/// @param[in] darray Dynamic array.
/// @param[in] item   Pointer to item to push.
/// @return
///     - @c True  : Had capacity to push new item.
///     - @c False : Dynamic array is full.
b32 darray_try_push( void* darray, const void* item );
/// @brief Attempt to emplace new item inside of dynamic array.
/// @param[in] darray Dynamic array.
/// @param[in] item   Pointer to item to emplace.
/// @param     at     Index to emplace at.
/// @return
///     - @c True  : Had capacity to emplace new item.
///     - @c False : Dynamic array is full.
b32 darray_try_emplace( void* darray, const void* item, usize at );
/// @brief Attempt to append array to end of dynamic array.
/// @param[in] darray Dynamic array.
/// @param     count  Number of items in @c items.
/// @param[in] items  Pointer to start of array to append.
/// @return
///     - @c True  : Had capacity to append array.
///     - @c False : Dynamic array is full.
b32 darray_try_append( void* darray, usize count, const void* items );
/// @brief Attempt to insert array inside of dynamic array.
/// @param[in] darray Dynamic array.
/// @param     count  Number of items in @c items.
/// @param[in] items  Pointer to start of array to insert.
/// @param     at     Index to insert at.
/// @return
///     - @c True  : Had capacity to insert array.
///     - @c False : Dynamic array is full.
b32 darray_try_insert( void* darray, usize count, const void* items, usize at );
/// @brief Pop last item from dynamic array.
/// @param[in]  darray       Dynamic array to pop from.
/// @param[out] opt_out_item (optional) Pointer to write last item to.
/// @return
///     - @c True  : Popped last item.
///     - @c False : Dynamic array is empty.
b32 darray_pop( void* darray, void* opt_out_item );
/// @brief Push new item to end of dynamic array.
/// @param[in] darray Dynamic array.
/// @param[in] item   Pointer to item to push.
/// @return
///     - @c Pointer : Pushed successfully.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_push( void* darray, const void* item );
/// @brief Emplace new item inside of dynamic array.
/// @param[in] darray Dynamic array.
/// @param[in] item   Pointer to item to emplace.
/// @param     at     Index to emplace at.
/// @return
///     - @c Pointer : Emplaced successfully.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_emplace( void* darray, const void* item, usize at );
/// @brief Append array to end of dynamic array.
/// @param[in] darray Dynamic array.
/// @param     count  Number of items in @c items.
/// @param[in] items  Pointer to start of array to append.
/// @return
///     - @c Pointer : Appended successfully.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_append( void* darray, usize count, const void* items );
/// @brief Insert array inside of dynamic array.
/// @param[in] darray Dynamic array.
/// @param     count  Number of items in @c items.
/// @param[in] items  Pointer to start of array to insert.
/// @param     at     Index to insert at.
/// @return
///     - @c Pointer : Inserted successfully.
///     - @c NULL    : Failed to reallocate dynamic array.
void* darray_insert( void* darray, usize count, const void* items, usize at );
/// @brief Remove item from dynamic array.
/// @param[in] darray Dynamic array.
/// @param     index  Index of item to remove.
/// @return
///     - @c True  : Removed item.
///     - @c False : Dynamic array was empty.
b32 darray_remove( void* darray, usize index );
/// @brief Remove range of items from dynamic array.
/// @param[in] darray Dynamic array.
/// @param     from_inclusive Start of range, inclusive.
/// @param     to_exclusive   End of range, exclusive. (can be == length).
/// @return
///     - @c True  : Range was in bounds of @c darray and was removed.
///     - @c False : Range was out of bounds.
b32 darray_remove_range( void* darray, usize from_inclusive, usize to_exclusive );
/// @brief Calculate remaining capacity in dynamic array.
/// @param[in] darray Dynamic array.
/// @return Remaining capacity.
usize darray_remaining( const void* darray );
/// @brief Get length of dynamic array.
/// @param[in] darray Dynamic array.
/// @return Length of dynamic array.
usize darray_len( const void* darray );
/// @brief Get capacity of dynamic array.
/// @param[in] darray Dynamic array.
/// @return Capacity of dynamic array.
usize darray_cap( const void* darray );
/// @brief Get size of items in dynamic array.
/// @param[in] darray Dynamic array.
/// @return Size of items in bytes.
usize darray_stride( const void* darray );
/// @brief Get total heap memory usage of dynamic array.
/// @param[in] darray Dynamic array.
/// @return Total memory usage.
usize darray_total_size( const void* darray );
/// @brief Check if dynamic array is empty.
/// @param[in] darray Dynamic array.
/// @return
///     - @c True  : Dynamic array is empty.
///     - @c False : Dynamic array has items in it.
b32 darray_is_empty( const void* darray );
/// @brief Check if dynamic array is full.
/// @param[in] darray Dynamic array.
/// @return
///     - @c True  : Dynamic array is full.
///     - @c False : Dynamic array has capacity available.
b32 darray_is_full( const void* darray );
/// @brief Get mutable pointer to start of dynamic array buffer.
/// @details
/// Used internally.
/// Returns NULL if @c darray is NULL.
/// @param[in] darray Dynamic array.
/// @return Start of dynamic array buffer.
void* darray_head( void* darray );
/// @brief Get pointer to start of dynamic array buffer.
/// @details
/// Used internally.
/// Returns NULL if @c darray is NULL.
/// @param[in] darray Dynamic array.
/// @return Start of dynamic array buffer.
const void* darray_head_const( const void* darray );
/// @brief Free a dynamic array.
/// @details
/// Use this function instead of memory_free().
/// @param[in] darray Dynamic array.
void darray_free( void* darray );

/// @brief Get read-only string containing current working directory.
/// @return Read-only string containing current working directory.
String path_cwd(void);
/// @brief Get read-only string containing home directory.
/// @return Read-only string containing home directory.
String path_home(void);
/// @brief Check if path is absolute.
/// @param[in] path Path to check.
/// @return
///     - @c True  : @c path is an absolute path.
///     - @c False : @c path is a relative path.
b32 path_is_absolute( const char* path );
/// @brief Check if path points to a file/directory.
/// @details
/// Untested: if it returns false for files that require elevated permissions.
/// @param[in] path Path to check.
/// @return
///     - @c True  : @c path points to real file/directory.
///     - @c False : @c path does not point to file/directory.
b32 path_exists( const char* path );
/// @brief Check if path points to directory.
/// @param[in] path Path to check.
/// @return
///     - @c True  : @c path points to existing directory.
///     - @c False : @c path points to something else.
///     - @c False : @c path does not point to existing directory.
b32 path_is_directory( const char* path );
/// @brief Count number of path chunks in path.
/// @param path Path to count.
/// @return Number of chunks.
usize path_chunk_count( String path );
/// @brief Split path into chunks.
/// @param path Path to split.
/// @return
///     - Dynamic array of chunks. Free with darray_free()
///     - NULL : Failed to allocate chunks array.
String* path_chunk_split( String path );
/// @brief Check if path matches glob pattern.
/// @details
/// This is a very simple glob pattern.
/// It only supports:
///
/// - * : One or more any characters.
/// - ? : One of any character.
/// @param path Path to check.
/// @param glob Glob pattern.
/// @return
///     - @c True  : @c path matches @c glob.
///     - @c False : @c path does not match @c glob.
b32 path_matches_glob( String path, String glob );
/// @brief Walk a directory, collecting all files/directories.
/// @details
/// If @c out_result is empty, allocates new buffers for it.
/// Otherwise, appends results to end of @c out_result.
/// @param[in]      dir          Path to directory to walk.
/// @param          recursive    If function should keep searching in subdirectories.
/// @param          include_dirs If function should include directory names in final list.
/// @param[in, out] out_result   Result of directory walk.
/// @return
///     - @c True  : Successfully walked directory and wrote results to @c out_result.
///     - @c False : Failed to open @c dir.
///     - @c False : Failed to allocate results.
b32 path_walk_dir(
    const char* dir, b32 recursive,
    b32 include_dirs, WalkDirectory* out_result );
/// @brief Create list of filtered paths from path_walk_dir() result.
/// @param[in] wd   path_walk_dir() result to filter.
/// @param     glob Glob pattern to check for.
/// @return
///     - Dynamic array of paths that match pattern.
///     - NULL : Failed to allocate result.
Darray(String) path_walk_glob_filter( const WalkDirectory* wd, String glob );
/// @brief Free result of path_walk_dir().
/// @param[in] wd Walk directory result to free.
void path_walk_free( WalkDirectory* wd );

/// @brief Create a null file descriptor.
/// @return Null file descriptor.
#define file_null() (0)
/// @brief Open a file.
/// @param[in]  path     Path of file to open. Length must be <= 4096.
/// @param      flags    Flags to open with.
/// @param[out] out_file File descriptor if successful.
/// @return
///     - @c True  : File was opened successfully.
///     - @c False : Failed to open file. Check log for more details.
b32 fd_open( const char* path, FileOpenFlags flags, FD* out_file );
/// @brief Close a file descriptor.
/// @param[in] file Pointer to file descriptor to close.
void fd_close( FD* file );
/// @brief Write to file.
/// @param[in]  file               File descriptor to write to.
/// @param      size               Size of buffer to write.
/// @param[in]  buf                Pointer to buffer to write.
/// @param[out] opt_out_write_size (optional) Pointer to write number of bytes written.
/// @return
///     - @c True  : Successfully wrote to file.
///     - @c False : Failed to write file.
b32 fd_write( FD* file, usize size, const void* buf, usize* opt_out_write_size );
/// @brief Write formatted string to file.
/// @param[in] file   File descriptor to write to.
/// @param[in] format Format string literal.
/// @param     va     Variadic list of format arguments.
/// @return
///     - @c True  : Successfully wrote to file.
///     - @c False : Failed to write file.
b32 fd_write_fmt_va( FD* file, const char* format, va_list va );
/// @brief Write formatted string to file.
/// @param[in] file   File descriptor to write to.
/// @param[in] format Format string literal.
/// @param     ...    Format arguments.
/// @return
///     - @c True  : Successfully wrote to file.
///     - @c False : Failed to write file.
b32 fd_write_fmt( FD* file, const char* format, ... );
/// @brief Read bytes from file.
/// @param[in]  file              File descriptor to read from.
/// @param      size              Size of read buffer.
/// @param[out] buf               Pointer to buffer.
/// @param[out] opt_out_read_size (optional) Pointer to write number of bytes read.
/// @return
///     - @c True  : Successfully read from file.
///     - @c False : Failed to read file.
b32 fd_read( FD* file, usize size, void* buf, usize* opt_out_read_size );
/// @brief Set file size to current position.
/// @param[in] file File descriptor.
/// @return
///     - @c True  : Successfully set file size.
///     - @c False : Failed to set file size.
b32 fd_truncate( FD* file );
/// @brief Query file size.
/// @param[in] file File descriptor.
/// @return Size of @c file.
usize fd_query_size( FD* file );
/// @brief Seek to file position.
/// @param[in] file File descriptor.
/// @param     type Type of seek.
/// @param     seek Bytes to seek. Can be negative to seek in reverse.
void fd_seek( FD* file, FileSeek type, isize seek );
/// @brief Query current seek position.
/// @param[in] file File descriptor.
/// @return Bytes from start of file.
usize fd_query_position( FD* file );
/// @brief Query creation time of file at given path.
/// @param[in] path Null-terminated path to file. Length must be <= 4096.
/// @return Creation time in POSIX time.
time_t file_query_time_create( const char* path );
/// @brief Query last time modified of file at given path.
/// @param[in] path Null-terminated path to file. Length must be <= 4096.
/// @return Last time modified in POSIX time.
time_t file_query_time_modify( const char* path );
/// @brief Move file from one path to another.
/// @param[in] dst Null-terminated destination path. Length must be <= 4096.
/// @param[in] src Null-terminated source path. Length must be <= 4096.
/// @return
///     - @c True  : Successfully moved file.
///     - @c False : Failed to move file.
b32 file_move( const char* dst, const char* src );
/// @brief Copy file from one path to another.
/// @param[in] dst Null-terminated destination path. Length must be <= 4096.
/// @param[in] src Null-terminated source path. Length must be <= 4096.
/// @return
///     - @c True  : Successfully copied file.
///     - @c False : Failed to copie file.
b32 file_copy( const char* dst, const char* src );
/// @brief Remove file from system.
/// @param[in] path Null-terminated path. Length must be <= 4096.
/// @return
///     - @c True  : Successfully removed file.
///     - @c False : Failed to remove file.
b32 file_remove( const char* path );
/// @brief Create directory.
/// @param[in] path Path to create directory at.
/// @return
///     - @c True  : Directory created successfully or it already exists.
///     - @c False : Failed to create directory.
b32 dir_create( const char* path );
/// @brief Remove directory.
/// @param[in] path      Path to directory to remove.
/// @param     recursive If directory contains items.
/// @return
///     - @c True  : Directory removed successfully.
///     - @c False : Failed to remove directory.
///     - @c False : Attempted to remove directory that is not empty without @c recursive.
b32 dir_remove( const char* path, b32 recursive );

/// @brief Atomically add to atomic integer.
/// @param[in, out] atomic Pointer to atomic to add to.
/// @param          val    Value to add to @c atomic.
/// @return Value of @c atomic before add.
atom   atomic_add( atom* atomic, atom val );
/// @brief Atomically add to atomic integer.
/// @param[in, out] atom Pointer to atomic to add to.
/// @param          val    Value to add to @c atom.
/// @return Value of @c atom before add.
atom64 atomic_add64( atom64* atom, atom64 val );
/// @brief Compare atomic to value and if they match, exchange with new value.
/// @param[in, out] atomic Pointer to atomic to modify.
/// @param          comp   Value to compare agains @c atomic.
/// @param          exch   Value to exchange with @c atomic if comparison succeeds.
/// @return Value of @c atom before exchange.
atom   atomic_compare_swap( atom* atomic, atom comp, atom exch );
/// @brief Compare atomic to value and if they match, exchange with new value.
/// @param[in, out] atom Pointer to atomic to modify.
/// @param          comp Value to compare agains @c atom.
/// @param          exch Value to exchange with @c atom if comparison succeeds.
/// @return Value of @c atom before exchange.
atom64 atomic_compare_swap64( atom64* atom, atom64 comp, atom64 exch );
/// @brief Insert a full memory barrier.
/// @details
/// Inserts a compiler and instruction memory barrier to ensure memory ordering.
void fence(void);

#if defined(COMPILER_MSVC)
    /// @brief Create an unintialized mutex.
    /// @return Unintialized mutex.
    #define mutex_null() {0}
#else
    /// @brief Create an unintialized mutex.
    /// @return Unintialized mutex.
    #define mutex_null() (Mutex){ .handle=NULL }
#endif
/// @brief Create a mutex.
/// @param[out] out_mutex Pointer to write new mutex to.
/// @return
///     - @c True  : Created mutex successfully.
///     - @c False : Failed to create mutex (Windows only).
b32 mutex_create( Mutex* out_mutex );
/// @brief Check if mutex was initialized.
/// @param[in] mutex Pointer to mutex to check.
/// @return
///     - @c True : Mutex was initialized.
///     - @c False : Mutex has not yet been initialized (Windows only).
b32 mutex_is_valid( const Mutex* mutex );
/// @brief Lock mutex.
/// @details
/// Waits indefinitely for mutex lock.
/// @param[in] mutex Mutex to lock.
void mutex_lock( Mutex* mutex );
/// @brief Lock mutex, wait for only a limited time.
/// @param[in] mutex Mutex to lock.
/// @param     ms    Maximum milliseconds to wait for lock.
/// @return
///     - @c True  : Locked mutex before timeout.
///     - @c False : Timed out.
b32 mutex_lock_timed( Mutex* mutex, u32 ms );
/// @brief Unlock locked mutex. Does nothing if mutex is already unlocked.
/// @param[in] mutex Mutex to unlock.
void mutex_unlock( Mutex* mutex );
/// @brief Destroy mutex.
/// @param[in] mutex Mutex to destroy.
void mutex_destroy( Mutex* mutex );

#if defined(COMPILER_MSVC)
    /// @brief Create an uninitialized semaphore.
    /// @return Unintialized semaphore.
    #define semaphore_null() {0}
#else
    /// @brief Create an uninitialized semaphore.
    /// @return Unintialized semaphore.
    #define semaphore_null() (Semaphore){ .handle=NULL }
#endif
/// @brief Create a semaphore.
/// @param[out] out_semaphore Pointer to write semaphore to.
/// @return
///     - @c True  : Created semaphore successfully.
///     - @c False : Failed to create semaphore.
b32 semaphore_create( Semaphore* out_semaphore );
/// @brief Check if semaphore has already been initialized.
/// @param[in] semaphore Semaphore to check.
/// @return
///     - @c True  : Semaphore is initialized.
///     - @c False : Semaphore has not been initialized.
b32 semaphore_is_valid( const Semaphore* semaphore );
/// @brief Wait indefinitely for semaphore to be signaled.
/// @param[in] semaphore Semaphore to wait for.
void semaphore_wait( Semaphore* semaphore );
/// @brief Wait for semaphore to be signaled.
/// @param[in] semaphore Semaphore to wait for.
/// @param     ms        Maximum milliseconds to wait for signal.
/// @return
///     - @c True  : Semaphore was signaled in time.
///     - @c False : Timed out.
b32 semaphore_wait_timed( Semaphore* semaphore, u32 ms );
/// @brief Signal a semaphore.
/// @param[in] semaphore Semaphore to signal.
void semaphore_signal( Semaphore* semaphore );
/// @brief Destroy a semaphore.
/// @param[in] semaphore Semaphore to destroy.
void semaphore_destroy( Semaphore* semaphore );
/// @brief Sleep the current thread for given milliseconds.
/// @param ms Milliseconds to sleep for.
void thread_sleep( u32 ms );

/// @brief Enqueue a new job.
/// @param[in] job    Pointer to job function to enqueue.
/// @param[in] params (optional) Parameters for job function.
/// @return
///     - @c True  : Was able to enqueue job.
///     - @c False : Job queue is full, use job_enqueue_timed() instead to wait for empty spot.
b32 job_enqueue( JobFN* job, void* params );
/// @brief Wait for job queue to enqueue.
/// @param[in] job    Pointer to job function to enqueue.
/// @param[in] params (optional) Parameters for job function.
/// @param     ms     Maximum milliseconds to wait, use MT_WAIT_INFINITE to wait indefinitely.
/// @return
///     - @c True  : Was able to enqueue in time.
///     - @c False : Timed out.
b32 job_enqueue_timed( JobFN* job, void* params, u32 ms );
/// @brief Wait for next job to complete.
/// @param ms Milliseconds to wait for, use MT_WAIT_INFINITE to wait indefinitely.
/// @return
///      - @c True  : Next job completed in time or queue was empty.
///      - @c False : Timed out.
b32 job_wait_next( u32 ms );
/// @brief Wait for all jobs to complete.
/// @param ms Milliseconds to wait for, use MT_WAIT_INFINITE to wait indefinitely.
/// @return
///      - @c True  : Jobs completed in time or queue was empty.
///      - @c False : Timed out.
b32 job_wait_all( u32 ms );

/// @brief Get the current thread's monotonic id.
/// @return Current thread id. (0 is main thread)
u32 thread_id(void);

#if defined(COMPILER_MSVC)
    /// @brief Create empty command.
    /// @return Empty command.
    #define command_null() {0}
#else
    /// @brief Create empty command.
    /// @return Empty command.
    #define command_null() (Command){ .args=0, .count=0 }
#endif
/// @brief Create new command.
/// @param ... Null-terminated strings of arguments. First argument must be path to process to execute.
/// @return New command.
#define command_new( ... )\
    (Command){ .args=(const char*[]){ __VA_ARGS__, 0 },\
        .count=sizeof((const char*[]){ __VA_ARGS__ }) / sizeof(const char*) }
/// @brief Convert command to dynamic string.
/// @param[in] command Command to convert.
/// @return
///     - Dynamic string, free with dstring_free().
///     - NULL : Failed to allocate result.
DString* command_flatten_dstring( const Command* command );

/// @brief Create command builder.
/// @param[in]  exe         Name of executable.
/// @param[out] out_builder Pointer to write result to.
/// @return
///     - true  : Allocated new command builder.
///     - false : Failed to allocate new command builder.
b32 command_builder_new( const char* exe, CommandBuilder* out_builder );
/// @brief Clear command builder.
/// @param[in] builder Builder to clear.
void command_builder_clear( CommandBuilder* builder );
/// @brief Push argument to end of command builder.
/// @param[in] builder Builder to push to.
/// @param[in] arg     Argument to push.
/// @return
///     - true  : Pushed argument successfully.
///     - false : Failed to reallocate command builder.
b32 command_builder_push( CommandBuilder* builder, const char* arg );
b32 __internal_command_builder_append( CommandBuilder* builder, const char* first, ... );
/// @brief Append arguments to end of command builder.
/// @param[in] builder Builder to append to.
/// @param     args... Arguments to append.
/// @return
///     - true  : Appended arguments successfully.
///     - false : Failed to reallocate command builder.
#define command_builder_append( builder, ... )\
    __internal_command_builder_append( builder, __VA_ARGS__, NULL )
/// @brief Append list of arguments to end of command builder.
/// @param[in] builder Builder to append to.
/// @param     count   Number of arguments to append.
/// @param[in] args    Pointer to list of arguments to append.
/// @return
///     - true  : Appended arguments successfully.
///     - false : Failed to reallocate command builder.
b32 command_builder_append_list(
    CommandBuilder* builder, usize count, const char** args );
/// @brief Convert builder to command.
/// @param[in] builder Builder to convert.
/// @return Command.
Command command_builder_cmd( CommandBuilder* builder );
/// @brief Free command builder.
/// @param[in] builder Builder to free.
void command_builder_free( CommandBuilder* builder );

#if defined(PLATFORM_WINDOWS)
    /// @brief Create null pipe.
    /// @return Null pipe.
    #define pipe_null() (0)
#else
    /// @brief Create null pipe.
    /// @return Null pipe.
    #define pipe_null() (-1)
#endif
/// @brief Open read and write pipes.
/// @param[out] out_read  Pointer to write read end of pipe to.
/// @param[out] out_write Pointer to write write end of pipe to.
void pipe_open( ReadPipe* out_read, WritePipe* out_write );
/// @brief Read from read end of pipe.
/// @param[in]  read_pipe         Pointer to read end of pipe.
/// @param      size              Size of buffer to read to.
/// @param[in]  buf               Buffer to write read result to.
/// @param[out] opt_out_read_size (optional) Pointer to write number of bytes read.
/// @return
///     - @c True  : Successfully read from pipe.
///     - @c False : Failed to read from pipe.
#define pipe_read( read_pipe, size, buf, opt_out_read_size )\
    fd_read( read_pipe, size, buf, opt_out_read_size )
/// @brief Write to write end of pipe.
/// @param[in]  write_pipe         Pointer to write end of pipe.
/// @param      size               Size of buffer to write.
/// @param[in]  buf                Buffer to write to pipe.
/// @param[out] opt_out_write_size (optional) Pointer to write number of bytes written.
/// @return
///     - @c True  : Successfully wrote to pipe.
///     - @c False : Failed to write to pipe.
#define pipe_write( write_pipe, size, buf, opt_out_write_size )\
    fd_write( write_pipe, size, buf, opt_out_write_size )
/// @brief Close pipe.
/// @param pipe Pipe to close (ReadPipe or WritePipe).
void pipe_close( Pipe pipe );

/// @brief Create null process ID.
/// @return Null process ID.
#define pid_null() (0)
/// @brief Check if process is in path.
/// @param[in] process_name Name of process to check for.
/// @return
///     - @c True  : Process is in path.
///     - @c False : Process is not in path.
b32 process_in_path( const char* process_name );
/// @brief Execute a process command.
/// @param cmd Command to execute.
/// @param redirect_void If output should just be redirected to void.
/// @param[in] opt_stdin  (optional) Pointer to read stdin pipe.
/// @param[in] opt_stdout (optional) Pointer to write stdout pipe.
/// @param[in] opt_stderr (optional) Pointer to write stderr pipe.
/// @param[in] opt_cwd    (optional) Change current working directory for command.
/// @return ID of process executed.
/// Process ID must be discarded using process_discard(),
/// process_wait() or successful process_wait_timed().
PID process_exec(
    Command cmd, b32 redirect_void, ReadPipe* opt_stdin,
    WritePipe* opt_stdout, WritePipe* opt_stderr, const char* opt_cwd );
/// @brief Wait indefinitely for process to complete.
/// @details
/// This function discards @c pid.
/// @param pid ID of process to wait for.
/// @return
///     - >= 0 : Process exited normally, return code from process.
///     - <  0 : Process did not exit normally.
int process_wait( PID pid );
/// @brief Wait for process to complete.
/// @details
/// If return code is >= 0 that means process exited normally.
/// If return code is < 0 that means process did not exit normally.
/// If process completes in time, this function discards @c pid.
/// @param      pid         ID of process to wait for.
/// @param[out] opt_out_res (optional) Pointer to write return code to.
/// @param      ms          Max milliseconds to wait for.
/// @return
///     - @c True  : Process completed in time.
///     - @c False : Timed out.
b32 process_wait_timed( PID pid, int* opt_out_res, u32 ms );
/// @brief Discard process ID.
/// @details
/// While not necessary on POSIX, pid should always
/// be discard with process_wait(), proces_wait_timed() or this function.
/// @param pid Process ID to discard.
void process_discard( PID pid );

/// @brief Get current time in milliseconds.
/// @return Time in milliseconds.
f64 timer_milliseconds(void);
/// @brief Get current time in seconds.
/// @return Time in seconds.
f64 timer_seconds(void);

/// @brief Get pointer to next local byte buffer.
/// @details
/// cbuild allocates CBUILD_LOCAL_BUFFER_COUNT * CBUILD_THREAD_COUNT
/// number of buffers of size CBUILD_LOCAL_BUFFER_CAPACITY
/// when either init() or this function is first called.
/// Local buffers should not be freed.
/// Local buffers should be used immediately to prevent
/// other functions from overwriting the currently obtained
/// local buffer.
/// @return Pointer to next local byte buffer.
u8* local_byte_buffer(void);
/// @brief Write formatted string to local buffer.
/// @details
/// cbuild allocates CBUILD_LOCAL_BUFFER_COUNT * CBUILD_THREAD_COUNT
/// number of buffers of size CBUILD_LOCAL_BUFFER_CAPACITY
/// when either init() or this function is first called.
/// Local buffers should not be freed.
/// Local buffers should be used immediately to prevent
/// other functions from overwriting the currently obtained
/// local buffer.
/// @param[in] format Format string literal.
/// @param     va     Variadic list of format arguments.
/// @return Pointer to start of formatted local buffer.
char* local_fmt_va( const char* format, va_list va );
/// @brief Write formatted string to local buffer.
/// @details
/// cbuild allocates CBUILD_LOCAL_BUFFER_COUNT * CBUILD_THREAD_COUNT
/// number of buffers of size CBUILD_LOCAL_BUFFER_CAPACITY
/// when either init() or this function is first called.
/// Local buffers should not be freed.
/// Local buffers should be used immediately to prevent
/// other functions from overwriting the currently obtained
/// local buffer.
/// @param[in] format Format string literal.
/// @param     ...    Format arguments.
/// @return Pointer to start of formatted local buffer.
char* local_fmt( const char* format, ... );

/// @brief Set logging level.
/// @warning
/// This function is not MT-safe so only
/// call it before using jobs system.
/// @param level Level to set logger to.
void logger_set_level( LoggerLevel level );
/// @brief Get current logging level.
/// @return Current logging level.
LoggerLevel logger_get_level(void);
/// @brief Write formatted logging message.
/// @param     level  Logger level of message.
/// @param[in] format Format string literal.
/// @param     va     Variadic list of format arguments.
void logger_va( LoggerLevel level, const char* format, va_list va );
/// @brief Write formatted logging message.
/// @param     level  Logger level of message.
/// @param[in] format Format string literal.
/// @param     ...    Format arguments.
void logger( LoggerLevel level, const char* format, ... );

/// @brief Query name of compiler used to compile this instance of cbuild.
/// @details
/// This name corresponds to the command used to compiler cbuild.
/// @return String containing name of compiler.
String cbuild_query_compiler(void);
/// @brief Query command line arguments used in this instance of cbuild.
/// @return Pointer to command.
const Command* cbuild_query_command_line(void);

/// @brief Log an info level message to stdout.
/// @param ... (format and format arguments) Message to log.
#define cb_info( ... )  logger( LOGGER_LEVEL_INFO, __VA_ARGS__ )
/// @brief Log a warning level message to stdout.
/// @param ... (format and format arguments) Message to log.
#define cb_warn( ... )  logger( LOGGER_LEVEL_WARNING, __VA_ARGS__ )
/// @brief Log an error level message to stderr.
/// @param ... (format and format arguments) Message to log.
#define cb_error( ... ) logger( LOGGER_LEVEL_ERROR, __VA_ARGS__ )
/// @brief Log a fatal level message to stderr.
/// @param ... (format and format arguments) Message to log.
#define cb_fatal( ... ) logger( LOGGER_LEVEL_FATAL, __VA_ARGS__ )

#if defined(CBUILD_ASSERTIONS)
    /// @brief Check if condition is true. Panic if it's not. (Noreturn on fail)
    /// @param condition (boolean expression) Condition to check.
    /// @param ...       (format and format arguments) Message to log upon failed condition.
    #define assertion( condition, ... ) do {\
        if( !(condition) ) {\
            fprintf( stderr, "\033[1;35m[F:%02u] "__FILE__":%i:%s(): assertion '"#condition"' failed! message: ", thread_id(), __LINE__, __FUNCTION__ );\
            fprintf( stderr, __VA_ARGS__ );\
            fprintf( stderr, "\033[1;00m\n" );\
            fflush( stderr );\
            fence();\
            __insert_panic();\
        }\
    } while(0)
#else
    /// @brief Check if condition is true. Panic if it's not. (Noreturn on fail)
    /// @param ... (anything) Assertion is disabled so any arguments go to unused() macro.
    #define assertion( ... ) unused( __VA_ARGS__ )
#endif

/// @brief Insert a panic with message. (Noreturn)
/// @param ... (format and format arguments) Message to log.
#define panic( ... ) do {\
    fprintf( stderr, "\033[1;35m[F:%02u] "__FILE__":%i:%s(): panic! message: ", thread_id(), __LINE__, __FUNCTION__ );\
    fprintf( stderr, __VA_ARGS__ );\
    fprintf( stderr, "\033[1;00m\n" );\
    fflush( stderr );\
    fence();\
    __insert_panic();\
} while(0)

/// @brief Assert something that should always be checked. (Noreturn on fail)
/// @details
/// Crashes program rather than inserting exit(-1) on fail.
/// @param condition (boolean expression) Condition to check.
/// @param ...       (format and format arguments) Message to log upon failed condition.
#define expect_crash( condition, ... ) do {\
    if( !(condition) ) {\
        fprintf( stderr, "\033[1;35m[F:%02u] " __FILE__ ":%i:%s(): expected '" #condition "'! message: ", thread_id(), __LINE__, __FUNCTION__ );\
        fprintf( stderr, __VA_ARGS__ );\
        fprintf( stderr, "\033[1;00m\n" );\
        fflush( stderr );\
        fence();\
        __insert_crash();\
    }\
} while(0)

/// @brief Assert something that should always be checked. (Noreturn on fail)
/// @param condition (boolean expression) Condition to check.
/// @param ...       (format and format arguments) Message to log upon failed condition.
#define expect( condition, ... ) do {\
    if( !(condition) ) {\
        fprintf( stderr, "\033[1;35m[F:%02u] " __FILE__ ":%i:%s(): expected '" #condition "'! message: ", thread_id(), __LINE__, __FUNCTION__ );\
        fprintf( stderr, __VA_ARGS__ );\
        fprintf( stderr, "\033[1;00m\n" );\
        fflush( stderr );\
        fence();\
        __insert_panic();\
    }\
} while(0)

/// @brief Mark control path as unimplemented. (Noreturn)
#define unimplemented() do {\
    fprintf( stderr, "\033[1;35m[F:%02u] " __FILE__ ":%i:%s(): unimplemented path!\033[1;00m\n", thread_id(), __LINE__, __FUNCTION__ );\
    fflush( stderr );\
    fence();\
    __insert_panic();\
} while(0)

/// @brief Mark control path as unreachable (hopefully). (Noreturn)
#define unreachable() do {\
    fprintf( stderr, "\033[1;35m[F:%02u] " __FILE__ ":%i:%s(): reached unreachable path!\033[1;00m\n", thread_id(), __LINE__, __FUNCTION__ );\
    fflush( stderr );\
    fence();\
    __insert_unreachable();\
    __insert_panic();\
} while(0)
/*           ^^^^^ just in case compiler warns */

void _init_(
    LoggerLevel logger_level,
    const char* executable_name,
    const char* source_name,
    int argc, const char** argv );

#endif /* header guard */

#if defined(CBUILD_IMPLEMENTATION)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#if defined(PLATFORM_WINDOWS)
    #include <process.h>
#else
    void posix_process_replace( Command cmd );
#endif

struct GlobalBuffers {
    atom obtained; // atomic boolean

    String cwd;
    String home;

    ReadPipe  void_read;
    WritePipe void_write;
};
struct LocalBuffers {
    char buffers[CBUILD_THREAD_COUNT + 1]
        [CBUILD_LOCAL_BUFFER_COUNT][CBUILD_LOCAL_BUFFER_CAPACITY];
    atom indices[CBUILD_THREAD_COUNT + 1];
};
struct DynamicString {
    usize len, cap;
    char  buf[];
};
struct DynamicArray {
    usize len, cap, stride;
    u8 buf[];
};

struct JobEntry {
    JobFN* proc;
    void*  params;
};
struct JobQueue {
    Semaphore wakeup;
    atom front, back;
    atom pending, len;
    struct JobEntry entries[CBUILD_MAX_JOBS];
};

volatile struct JobQueue* global_queue = NULL;

atom   global_is_mt              = false; // boolean
atom64 global_memory_usage       = 0;
atom64 global_total_memory_usage = 0;

atom            global_thread_id_source = 1; // 0 is main thread
make_tls( u32 ) global_thread_id        = 0;

static Mutex       global_logger_mutex = mutex_null();
static LoggerLevel global_logger_level = LOGGER_LEVEL_INFO;

static Command global_command_line;

volatile struct LocalBuffers* global_local_buffers = NULL;
#if defined(COMPILER_MSVC)
    volatile struct GlobalBuffers global_buffers = {0};
#else
    volatile struct GlobalBuffers global_buffers =
        (struct GlobalBuffers){ .obtained=false };
#endif

void  thread_create( JobFN* func, void* params );
char* internal_cwd(void);
char* internal_home(void);
static b32 path_walk_dir_internal(
    DString** path, b32 recursive, b32 include_dirs,
    usize* out_count, DString** out_buffer );

static b32 job_dequeue( struct JobQueue* queue, struct JobEntry* out_entry ) {
    if( !queue->len ) {
        return false;
    }
    atomic_add( &queue->len, -1 );

    fence();
    atom front = atomic_add( &queue->front, 1 );
    fence();

    *out_entry = queue->entries[ (front + 1) % CBUILD_MAX_JOBS ];
    return true;
}
void job_queue_proc( void* params ) {
    struct JobQueue* queue = params;
    fence();

    for( ;; ) {
        struct JobEntry entry;
        memory_zero( &entry, sizeof(entry) );

        semaphore_wait( &queue->wakeup );
        fence();

        if( job_dequeue( queue, &entry ) ) {
            entry.proc( entry.params );
            fence();
            atomic_add( &queue->pending, -1 );
        }
    }
}
static void initialize_job_queue(void) {
    cb_info(
        "creating job queue with %u entries and %u threads . . .",
        CBUILD_MAX_JOBS, CBUILD_THREAD_COUNT );

    expect( mutex_create( &global_logger_mutex ), "failed to create logger mutex!" );
    atomic_add( &global_is_mt, 1 );

    usize queue_size       = sizeof(*global_queue);
    struct JobQueue* queue = memory_alloc( queue_size );

    expect( queue, "failed to allocate job queue!" );

    expect(
        semaphore_create( &queue->wakeup ),
        "failed to create job queue semaphore!" );
    queue->front = -1;

    fence();

    for( usize i = 0; i < CBUILD_THREAD_COUNT; ++i ) {
        thread_create( job_queue_proc, queue );
    }

    fence();
    global_queue = queue;
}
static volatile struct JobQueue* get_job_queue(void) {
    if( !global_queue ) {
        initialize_job_queue();
    }
    return global_queue;
}
static volatile struct LocalBuffers* get_local_buffers(void) {
    if( !global_local_buffers ) {
        global_local_buffers = memory_alloc( sizeof(*global_local_buffers) );
        expect( global_local_buffers,
            "failed to allocate local buffers! size: %zu",
            sizeof(*global_local_buffers) );
    }
    return global_local_buffers;
}
static volatile char* get_next_local_buffer( u32 id ) {
    volatile struct LocalBuffers* b = get_local_buffers();

    u32 index = 0;
    if( global_is_mt ) {
        atom atomic = atomic_add( &b->indices[id], 1 );
        index = (*(u32*)&atomic) % CBUILD_LOCAL_BUFFER_COUNT;
    } else {
        atom atomic     = b->indices[id];
        b->indices[id] += 1;
        index = (*(u32*)&atomic) % CBUILD_LOCAL_BUFFER_COUNT;
    }

    volatile char* result = b->buffers[id][index];
    memory_zero( (void*)result, CBUILD_LOCAL_BUFFER_CAPACITY );
    return result;
}
static volatile struct GlobalBuffers* get_global_buffers(void) {
    if( !global_buffers.obtained ) {
        atomic_add( &global_buffers.obtained, 1 );
        
        char* cwd  = internal_cwd();
        char* home = internal_home();
        global_buffers.cwd  = string_from_cstr( cwd );
        global_buffers.home = string_from_cstr( home );

        pipe_open(
            (ReadPipe*)&global_buffers.void_read,
            (WritePipe*)&global_buffers.void_write );
    }
    return &global_buffers;
}
static b32 validate_file_flags( FileOpenFlags flags ) {
    if( !( flags & FOPEN_READ | FOPEN_WRITE ) ) {
        cb_error( "FD flags must have READ and/or WRITE set!" );
        return false;
    }
    if( flags & FOPEN_TRUNCATE ) {
        if( flags & FOPEN_APPEND ) {
            cb_error( "FD flag APPEND and TRUNCATE cannot be set at the same time!" );
            return false;
        }
        if( flags & FOPEN_READ ) {
            cb_error( "FD flag TRUNCATE and READ cannot be set at the same time!" );
            return false;
        }
    }

    return true;
}
static b32 dir_remove_internal( const char* path );

#if !defined(CBUILD_COMPILER_NAME)
    #if defined(COMPILER_MSVC)
        #define CBUILD_COMPILER_NAME "cl"
    #elif defined(COMPILER_CLANG)
        #define CBUILD_COMPILER_NAME "clang"
    #elif defined(COMPILER_GCC)
        #define CBUILD_COMPILER_NAME "gcc"
    #else
        #define CBUILD_COMPILER_NAME "cc"
    #endif
#endif

#if !defined(CBUILD_COMPILER_OUTPUT_FLAG)
    #if defined(COMPILER_MSVC)
        #define CBUILD_COMPILER_OUTPUT_FLAG "-Fe:"
    #else
        #define CBUILD_COMPILER_OUTPUT_FLAG "-o"
    #endif
#endif

#if !defined(CBUILD_POSIX_FLAGS) && defined(PLATFORM_POSIX)
    #define CBUILD_POSIX_FLAGS "-lpthread"
#endif

String cbuild_query_compiler(void) {
    return string_text( CBUILD_COMPILER_NAME );
}
const Command* cbuild_query_command_line(void) {
    return &global_command_line;
}

void _platform_init_(void);
void _init_(
    LoggerLevel logger_level,
    const char* executable_name,
    const char* source_name,
    int argc, const char** argv
) {
    _platform_init_();
    logger_set_level( logger_level );

    const char** heap_args = (const char**)darray_from_array(
        sizeof(const char*), argc, argv
    ); {
        expect( heap_args, "failed to allocate arguments!" );
        const char* nul = 0;
        heap_args = darray_push( heap_args, &nul );
    }

    global_command_line.count = argc;
    global_command_line.args  = heap_args;

    (void)get_local_buffers();
    (void)get_global_buffers();

    expect( path_exists( __FILE__ ),
        "cbuild MUST be run from its source code directory!" );
    expect( path_exists( source_name ),
        "cbuild MUST be run from its source code directory!" );

    b32 rebuild = false;
    if( path_exists( executable_name ) ) {
        time_t executable_modify = file_query_time_modify( executable_name );
        time_t source_modify     = file_query_time_modify( source_name );
        time_t header_modify     = file_query_time_modify( __FILE__ );

        f64 diff_source = difftime( executable_modify, source_modify );
        f64 diff_header = difftime( executable_modify, header_modify );

        rebuild = (diff_source < 0.0) || (diff_header < 0.0);
    } else {
        rebuild = true;
    }

    if( !rebuild ) {
        const char* old_name = local_fmt( "%s.old", executable_name );
        if( path_exists( old_name ) ) {
            file_remove( old_name );
        }
        return;
    }
    // rebuild

    cb_info( "changes detected in cbuild source, rebuilding . . ." );
    cbuild_rebuild( source_name, executable_name, true );
}
does_not_return() void cbuild_rebuild(
    const char* cbuild_source_file_name,
    const char* cbuild_executable_name,
    b32 reload
) {

    f64 start = timer_milliseconds();

    #if !defined(CBUILD_ADDITIONAL_FLAGS)
        #define CBUILD_ADDITIONAL_FLAGS 0
    #endif
    #if !defined(CBUILD_POSIX_FLAGS)
        #define CBUILD_POSIX_FLAGS 0
    #endif

    const char* posix_flags[] = { CBUILD_POSIX_FLAGS };
    const char* additional[]  = { CBUILD_ADDITIONAL_FLAGS };
    usize arg_count = 0;
    const char* args[6 + static_array_len( additional ) + static_array_len( posix_flags )];
    memory_zero( (void*)args, sizeof(args) );

    args[arg_count++] = CBUILD_COMPILER_NAME;
    args[arg_count++] = cbuild_source_file_name;
    args[arg_count++] = CBUILD_COMPILER_OUTPUT_FLAG;
    args[arg_count++] = cbuild_executable_name;

#if defined(COMPILER_MSVC)
    args[arg_count++] = "-nologo";
#endif

    for( int i = 0; i < static_array_len( additional ); ++i ) {
        const char* a = additional[i];
        if( a ) {
            args[arg_count++] = additional[i];
        }
    }
    for( int i = 0; i < static_array_len( posix_flags ); ++i ) {
        const char* a = posix_flags[i];
        if( a ) {
            args[arg_count++] = posix_flags[i];
        }
    }

    Command rebuild_cmd;
    rebuild_cmd.count = arg_count;
    rebuild_cmd.args  = args;

    cb_info( "rebuilding with command:" );

    const char* old_name = local_fmt( "%s.old", cbuild_executable_name );
    if( path_exists( old_name ) ) {
        expect(
            file_remove( old_name ),
            "could not remove old executable!" );
    }

    expect(
        file_move( old_name, cbuild_executable_name ),
        "could not rename executable!" );

    fence();

    PID pid = process_exec( rebuild_cmd, false, 0, 0, 0, 0 );
    int res = process_wait( pid );
    if( res != 0 ) {
        cb_fatal( "failed to rebuild!" );
        file_move( cbuild_executable_name, old_name );

        exit(127);
    }

#if defined(COMPILER_MSVC)
    /* attempt to remove annoying .obj file generated by msvc */ {
        String exe = string_from_cstr( cbuild_executable_name );
        char* local = (char*)local_byte_buffer();
        memory_copy( local, exe.cc, exe.len );
        usize dot = 0;
        if( string_find_rev( exe, '.', &dot ) ) {
            local[++dot] = 'o';
            local[++dot] = 'b';
            local[++dot] = 'j';
            if( path_exists( local ) ) {
                file_remove( local );
            }
        }
    }
#endif

    f64 end = timer_milliseconds();
    cb_info( "rebuilt in %fms", end - start );

    if( !reload ) {
        exit(0);
    }

#if defined(PLATFORM_WINDOWS)
    printf(
        "\033[1;33m"
        "[W:00] cbuild: "
        "windows does not support automatically reloading cbuild, "
        "please run it again."
        "\033[1;00m\n" );
    exit(0);
#else

    posix_process_replace( global_command_line );
    // execvp( global_command_line.args[0], (char* const*)(global_command_line.args) );
    // process_exec( global_command_line, false, 0, 0, 0, 0 );
    exit(0);
#endif
}

usize memory_query_usage(void) {
    return global_memory_usage;
}
usize memory_query_total_usage(void) {
    return global_total_memory_usage;
}
void memory_set( void* memory, i8 value, usize size ) {
    memset( memory, value, size );
}
void memory_zero( void* memory, usize size ) {
    memory_set( memory, 0, size );
}
void memory_stamp( void* memory, usize value_size, const void* value, usize size ) {
    u8*   dst = (u8*)memory;
    usize rem = size;
    while( rem ) {
        if( value_size > rem ) {
            break;
        }

        memory_copy( dst, value, value_size );
        dst += value_size;
    }
}
void memory_copy( void* restrict dst, const void* restrict src, usize size ) {
    memcpy( dst, src, size );
}
void memory_move( void* dst, const void* src, usize size ) {
    memmove( dst, src, size );
}
b32 memory_cmp( const void* a, const void* b, usize size ) {
    return memcmp( a, b, size ) == 0;
}

b32 char_in_set( char c, String set ) {
    for( usize i = 0; i < set.len; ++i ) {
        if( c == set.cc[i] ) {
            return true;
        }
    }
    return false;
}

usize strlen_utf8( const char* str ) {
    return string_len_utf8( string_from_cstr( str ) );
}

b32 string_is_empty( String str ) {
    return !(str.cc && str.len);
}
b32 string_is_null_terminated( String str ) {
    if( !str.cc[str.len] ) {
        return true;
    }
    if( str.len && !str.cc[str.len - 1] ) {
        return true;
    }
    return false;
}
b32 string_cmp( String a, String b ) {
    if( a.len != b.len ) {
        return false;
    }
    return memory_cmp( a.cc, b.cc, a.len );
}
b32 string_find( String str, char c, usize* opt_out_index ) {
    const char* ptr = memchr( str.cc, c, str.len );
    if( !ptr ) {
        return false;
    }
    if( opt_out_index ) {
        *opt_out_index = ptr - str.cc;
    }
    return true;
}
b32 string_find_rev( String str, char c, usize* opt_out_index ) {
    for( usize i = str.len; i-- > 0; ) {
        if( str.cc[i] == c ) {
            if( opt_out_index ) {
                *opt_out_index = i;
            }
            return true;
        }
    }
    return false;
}
b32 string_find_set( String str, String set, usize* opt_out_index ) {
    for( usize i = 0; i < set.len; ++i ) {
        if( string_find( str, set.cc[i], opt_out_index ) ) {
            return true;
        }
    }
    return false;
}
b32 string_find_set_rev( String str, String set, usize* opt_out_index ) {
    for( usize i = str.len; i-- > 0; ) {
        if( char_in_set( str.cc[i], set ) ) {
            if( opt_out_index ) {
                *opt_out_index = i;
            }
            return true;
        }
    }
    return false;
}
b32 string_find_string( String str, String substr, usize* opt_out_index ) {
    String rem = str;
    if( rem.len < substr.len ) {
        return false;
    }

    while( rem.len ) {
        usize start = 0;
        if( string_find( rem, substr.cc[0], &start ) ) {
            rem = string_advance_by( rem, start );
            if( rem.len < substr.len ) {
                return false;
            }

            if( string_cmp( string_truncate(rem, substr.len), substr ) ) {
                if( opt_out_index ) {
                    *opt_out_index = rem.cc - str.cc;
                }
                return true;
            }
            rem = string_advance( rem );
        } else {
            break;
        }
    }
    return false;
}
b32 string_find_string_rev( String str, String substr, usize* opt_out_index ) {
    if( str.len < substr.len ) {
        return false;
    }
    for( usize i = str.len; i-- > 0; ) {
        if( str.cc[i] != substr.cc[i] ) {
            continue;
        }
        if( str.len - i < substr.len ) {
            break;
        }
        String part = string_new( substr.len, str.cc + i );

        if( string_cmp( part, substr ) ) {
            if( opt_out_index ) {
                *opt_out_index = i;
            }
            return true;
        }
    }
    return false;
}
char* string_first( String str ) {
    if( string_is_empty( str ) ) {
        return NULL;
    }
    return (char*)str.cc;
}
char* string_last( String str ) {
    if( string_is_empty( str ) ) {
        return NULL;
    }
    return (char*)( str.cc + (str.len - 1) );
}
String string_advance( String str ) {
    if( string_is_empty( str ) ) {
        return str;
    }
    return string_new( str.len - 1, str.cc + 1 );
}
String string_advance_by( String str, usize stride ) {
    if( string_is_empty( str ) ) {
        return str;
    }
    if( str.len <= stride ) {
        return string_new( 0, str.cc + (str.len - 1) );
    }
    return string_new( str.len - stride, str.cc + stride );
}
String string_truncate( String str, usize max ) {
    return string_new( max > str.len ? str.len : max, str.cc );
}
String string_trim( String str, usize amount ) {
    if( amount >= str.len ) {
        return string_new( 0, str.cc );
    }
    return string_new( str.len - amount, str.cc );
}
String string_trim_leading_whitespace( String str ) {
    String res = str;
    while( res.len ) {
        if( isspace( *res.cc ) ) {
            res = string_advance( res );
        } else {
            break;
        }
    }
    return res;
}
String string_trim_trailing_whitespace( String str ) {
    String res = str;
    while( res.len ) {
        if( isspace( *string_last( res ) ) ) {
            res.len--;
        } else {
            break;
        }
    }
    return res;
}
String string_trim_surrounding_whitespace( String str ) {
    return string_trim_leading_whitespace( string_trim_trailing_whitespace( str ) );
}
void string_split_at(
    String src, usize at, b32 keep_split, String* opt_out_left, String* opt_out_right
) {
    expect( at <= src.len,
        "index provided is outside string bounds! at: %zu", at );

    if( opt_out_left ) {
        *opt_out_left = string_truncate( src, at );
    }
    if( opt_out_right ) {
        *opt_out_right = string_advance_by( src, at + (keep_split ? 0 : 1) );
    }
}
b32 string_split_char(
    String src, char c, b32 keep_split, String* opt_out_left, String* opt_out_right
) {
    usize at = 0;
    if( !string_find( src, c, &at ) ) {
        return false;
    }
    string_split_at( src, at, keep_split, opt_out_left, opt_out_right );
    return true;
}
String* string_split_delim( String src, String delim, b32 keep_delim ) {
    usize  count  = 0;
    String substr = src;
    while( substr.len ) {
        usize pos = 0;
        if( string_find_string( substr, delim, &pos ) ) {
            count++;
            substr = string_advance_by( substr, pos + delim.len );
        } else {
            count++;
            break;
        }
    }

    String* res = (String*)darray_empty( sizeof(String), count );
    expect( res, "failed to allocate string buffer!" );

    if( count == 1 ) {
        return (String*)darray_push( res, &src );
    }

    substr = src;
    if( keep_delim ) {
        while( substr.len ) {
            usize pos = 0;
            if( string_find_string( substr, delim, &pos ) ) {
                String chunk = substr;
                chunk.len = pos + delim.len;

                expect( darray_try_push( res, &chunk ),
                    "misallocated result!" );

                substr = string_advance_by( substr, chunk.len );
            } else {
                expect( darray_try_push( res, &substr ),
                    "misallocated result!" );
                break;
            }
        }
    } else {
        while( substr.len ) {
            usize pos = 0;
            if( string_find_string( substr, delim, &pos ) ) {
                String chunk = substr;
                chunk.len    = pos;

                expect( darray_try_push( res, &chunk ),
                    "misallocated result!" );

                substr = string_advance_by( substr, chunk.len + delim.len );
            } else {
                expect( darray_try_push( res, &substr ),
                    "misallocated result!" );
                break;
            }
        }
    }

    return res;
}
String* string_split_delim_ex(
    String src, String delim, b32 keep_delim,
    StringSplitDelimFilterFN* filter, void* params
) {
    expect( filter, "no filter function provided!" );

    usize  count  = 0;
    String substr = src;
    while( substr.len ) {
        usize pos = 0;
        if( string_find_string( substr, delim, &pos ) ) {
            count++;
            substr = string_advance_by( substr, pos + delim.len );
        } else {
            count++;
            break;
        }
    }

    String* res = (String*)darray_empty( sizeof(String), count );
    expect( res, "failed to allocate string buffer!" );

    if( count == 1 ) {
        String filtered = filter( 0, src, params );
        if( filtered.len ) {
            expect( darray_try_push( res, &filtered ),
                "misallocated result!" );
        }
        return res;
    }

    usize index = 0;
    substr = src;
    if( keep_delim ) {
        while( substr.len ) {
            usize pos = 0;
            if( string_find_string( substr, delim, &pos ) ) {
                String chunk = substr;
                chunk.len    = pos + delim.len;

                chunk = filter( index, chunk, params );

                if( chunk.len ) {
                    expect( darray_try_push( res, &chunk ),
                        "misallocated result!" );
                    index++;
                }

                substr = string_advance_by( substr, chunk.len );
            } else {
                String filtered = filter( index++, substr, params );

                if( filtered.len ) {
                    expect( darray_try_push( res, &filtered ),
                        "misallocated result!" );
                }
                break;
            }
        }
    } else {
        while( substr.len ) {
            usize pos = 0;
            if( string_find_string( substr, delim, &pos ) ) {
                String chunk = substr;
                chunk.len    = pos;

                chunk = filter( index, chunk, params );

                if( chunk.len ) {
                    expect( darray_try_push( res, &chunk ),
                        "misallocated result!" );
                    index++;
                }

                substr = string_advance_by( substr, chunk.len + delim.len );
            } else {
                String filtered = filter( index++, substr, params );

                if( filtered.len ) {
                    expect( darray_try_push( res, &filtered ),
                        "misallocated result!" );
                }
                break;
            }
        }
    }

    return res;

}

usize string_len_utf8( String str ) {
    const unsigned char* ucc = (const unsigned char*)str.cc;
    usize res = 0;
    for( usize i = 0; i < str.len; ++i ) {
        if( (ucc[i] & 0xC0) != 0x80 ) {
            res++;
        }
    }
    return res;
}

DString* dstring_empty( usize cap ) {
    usize capacity = cap ? cap : 1;
    struct DynamicString* res =
        (struct DynamicString*)memory_alloc( sizeof(*res) + capacity );
    res->cap = capacity;
    return res->buf;
}
DString* dstring_new( usize len, const char* str ) {
    struct DynamicString* res = 
        (struct DynamicString*)dstring_head( dstring_empty( len + 1 ) );
    if( !res ) {
        return NULL;
    }
    memory_copy( res->buf, str, len );
    res->len = len;
    return res->buf;
}
DString* dstring_fmt_va( const char* format, va_list va ) {
    va_list va2;
    va_copy( va2, va );

    int msg_len = vsnprintf( 0, 0, format, va2 );
    va_end( va2 );

    struct DynamicString* res = dstring_head( dstring_empty( msg_len + 8 ) );
    if( !res ) {
        return NULL;
    }

    vsnprintf( res->buf, res->cap, format, va );
    res->len = msg_len;

    return res->buf;
}
DString* dstring_fmt( const char* format, ... ) {
    va_list va;
    va_start( va, format );
    DString* res = dstring_fmt_va( format, va );
    va_end( va );
    return res;
}
DString* dstring_grow( DString* str, usize amount ) {
    struct DynamicString* res = dstring_head( str );
    usize old_size = sizeof(struct DynamicString) + res->cap;
    usize new_size = old_size + amount;

    res = memory_realloc( res, old_size, new_size );
    if( !res ) {
        return NULL;
    }

    res->cap += amount;
    return res->buf;
}
DString* dstring_clone( const DString* src ) {
    return dstring_from_string( string_from_dstring( src ) );
}
DString* dstring_concat( String lhs, String rhs ) {
    usize len        = lhs.len + rhs.len;
    usize total_size = len + 8;
    struct DynamicString* res = dstring_head( dstring_empty( total_size ) );

    memory_copy( res->buf, lhs.cc, lhs.len );
    memory_copy( res->buf + lhs.len, rhs.cc, rhs.len );

    res->len = len;

    return res->buf;
}
DString* dstring_concat_multi(
    usize count, const String* strings, String opt_separator
) {
    expect( count, "did not provide any strings!" );
    usize total_size = (count - 1) * opt_separator.len;
    for( usize i = 0; i < count; ++i ) {
        total_size += strings[i].len;
    }

    DString* res = dstring_empty( total_size + 1 );

    if( opt_separator.len ) {
        for( usize i = 0; i < count; ++i ) {
            res = dstring_append( res, strings[i] );
            if( i + 1 != count ) {
                res = dstring_append( res, opt_separator );
            }
        }
    } else {
        for( usize i = 0; i < count; ++i ) {
            res = dstring_append( res, strings[i] );
        }
    }

    return res;
}
DString* dstring_concat_multi_cstr(
    usize count, const char** strings, const char* opt_separator
) {
    expect( count, "did not provide any strings!" );
    usize seplen     = opt_separator ? strlen( opt_separator ) : 0;
    usize total_size = (count - 1) * seplen;
    for( usize i = 0; i < count; ++i ) {
        const char* current = strings[i];
        if( !current ) {
            continue;
        }
        total_size += strlen( current );
    }

    DString* res = dstring_empty( total_size + 1 );

    if( opt_separator && seplen ) {
        String sep = string_new( seplen, opt_separator );
        for( usize i = 0; i < count; ++i ) {
            const char* current = strings[i];
            if( !current ) {
                continue;
            }

            res = dstring_append_cstr( res, current );
            if( i + 1 != count ) {
                res = dstring_append( res, sep );
            }
        }
    } else {
        for( usize i = 0; i < count; ++i ) {
            const char* current = strings[i];
            if( !current ) {
                continue;
            }
            res = dstring_append_cstr( res, current );
        }
    }

    return res;
}
DString* dstring_append( DString* str, String append ) {
    struct DynamicString* res = dstring_head( str );

    if( res->len + append.len + 1 > res->cap ) {
        res = dstring_head( dstring_grow( res->buf, append.len + 8 ) );
        if( !res ) {
            return NULL;
        }
    }

    memory_copy( res->buf + res->len, append.cc, append.len );
    res->len += append.len;
    res->buf[res->len] = 0;
    return res->buf;
}
DString* dstring_prepend( DString* str, String prepend ) {
    struct DynamicString* res = dstring_head( str );

    if( res->len + prepend.len + 1 > res->cap ) {
        res = dstring_head( dstring_grow( res->buf, prepend.len + 8 ) );
        if( !res ) {
            return NULL;
        }
    }

    memory_move( res->buf + prepend.len, res->buf, res->len + 1 );
    memory_copy( res->buf, prepend.cc, prepend.len );

    res->len += prepend.len;
    return res->buf;
}
DString* dstring_insert( DString* str, String insert, usize at ) {
    if( at == 0 ) {
        return dstring_prepend( str, insert );
    }
    struct DynamicString* res = dstring_head( str );
    if( res->len && res->len - 1 == at ) {
        return dstring_append( str, insert );
    }
    if( at >= res->len ) {
        cb_warn(
            "dstring_insert: attempted to insert past dstring bounds! "
            "len: %zu index: %zu", res->len, at );
        return NULL;
    }

    if( res->len + insert.len + 1 > res->cap ) {
        res = dstring_head( dstring_grow( res->buf, insert.len + 8 ) );
        if( !res ) {
            return NULL;
        }
    }

    memory_move( res->buf + at + insert.len, res->buf + at, (res->len + 1) - at );
    memory_copy( res->buf + at, insert.cc, insert.len );
    res->len += insert.len;

    return res->buf;
}
DString* dstring_push( DString* str, char c ) {
    struct DynamicString* res = dstring_head( str );
    if( res->len + 2 >= res->cap ) {
        res = dstring_head( dstring_grow( str, 8 ) );
        if( !res ) {
            return NULL;
        }
    }

    res->buf[res->len++] = c;
    res->buf[res->len]   = 0;
    return res->buf;
}
DString* dstring_emplace( DString* str, char c, usize at ) {
    return dstring_insert( str, string_new( 1, &c ), at );
}
b32 dstring_pop( DString* str, char* opt_out_c ) {
    struct DynamicString* head = dstring_head( str );
    if( !head->len ) {
        return false;
    }
    char c = head->buf[--head->len];
    head->buf[head->len] = 0;

    if( opt_out_c ) {
        *opt_out_c = c;
    }
    return true;
}
b32 dstring_remove( DString* str, usize index ) {
    struct DynamicString* head = dstring_head( str );
    if( !head->len || index > head->len ) {
        cb_warn(
            "dstring_remove: attempted to remove past dstring bounds! "
            "len: %zu index: %zu", head->len, index );
        return false;
    }

    memory_move( head->buf + index, head->buf + index + 1, (head->len + 1) - index );
    head->len--;

    return true;
}
b32 dstring_remove_range( DString* str, usize from_inclusive, usize to_exclusive ) {
    assertion( from_inclusive < to_exclusive,
        "dstring_remove_range: invalid range provided! (%zu, %zu]",
        from_inclusive, to_exclusive );
    struct DynamicString* head = dstring_head( str );
    if( !head->len || from_inclusive >= head->len || to_exclusive > head->len ) {
        cb_warn(
            "dstring_remove_range: attempted to remove past dstring bounds! "
            "len: %zu range: (%zu, %zu]", head->len, from_inclusive, to_exclusive );
        return false;
    }

    usize span = to_exclusive - from_inclusive;

    memory_move(
        head->buf + from_inclusive,
        head->buf + to_exclusive,
        (head->len + 1) - span );
    head->len -= span;

    return true;
}
void dstring_truncate( DString* str, usize max ) {
    struct DynamicString* head = dstring_head( str );
    if( max >= head->len ) {
        return;
    }
    memory_zero( head->buf + max, head->len - max );
    head->len = max;
}
void dstring_trim( DString* str, usize amount ) {
    usize len = dstring_len( str );
    dstring_truncate( str, amount > len ? 0 : len - amount );
}
void dstring_clear( DString* str ) {
    struct DynamicString* head = dstring_head( str );

    memory_zero( head->buf, head->len );
    head->len = 0;
}
usize dstring_remaining( const DString* str ) {
    // -1 to not include null-terminator
    return (dstring_cap( str ) - 1)  - dstring_len( str );
}
usize dstring_len( const DString* str ) {
    return ((struct DynamicString*)str - 1)->len;
}
usize dstring_cap( const DString* str ) {
    return ((struct DynamicString*)str - 1)->cap;
}
usize dstring_total_size( const DString* str ) {
    return dstring_cap( str ) + sizeof(struct DynamicString);
}
b32 dstring_is_empty( const DString* str ) {
    return dstring_len( str ) == 0;
}
b32 dstring_is_full( const DString* str ) {
    return dstring_len( str ) == dstring_cap( str );
}
void dstring_set_len( DString* str, usize len ) {
    struct DynamicString* head = dstring_head( str );
    expect( len < head->cap, "length exceeds string capacity!" );
    head->len            = len;
    head->buf[head->len] = 0;
}
void* dstring_head( DString* str ) {
    if( !str ) {
        return NULL;
    }
    return str - sizeof(struct DynamicString);
}
const void* dstring_head_const( const DString* str ) {
    if( !str ) {
        return NULL;
    }
    return str - sizeof(struct DynamicString);
}
void dstring_free( DString* str ) {
    struct DynamicString* head = dstring_head( str );
    if( !head ) {
        return;
    }
    usize total_size = head->cap + sizeof(*head);
    memory_free( head, total_size );
}

String path_cwd(void) {
    volatile struct GlobalBuffers* gb = get_global_buffers();
    return gb->cwd;
}
String path_home(void) {
    volatile struct GlobalBuffers* gb = get_global_buffers();
    return gb->home;
}
usize path_chunk_count( String path ) {
    String subpath = path;
    usize  count   = 0;

    while( subpath.len ) {
        usize sep = 0;
        if( string_find( subpath, '/', &sep ) ) {
            String chunk = subpath;
            chunk.len    = sep;

            subpath = string_advance_by( subpath, sep + 1 );
            count++;
        } else {
            count++;
            break;
        }
    }

    return count;
}
String* path_chunk_split( String path ) {
    usize   cap = path_chunk_count( path );
    String* res = darray_empty( sizeof(*res), cap ? cap : 1 );
    if( !res ) {
        return NULL;
    }

    String subpath = path;
    while( subpath.len ) {
        usize sep = 0;
        if( string_find( subpath, '/', &sep ) ) {
            String chunk = subpath;
            chunk.len    = sep;

            expect( darray_try_push( res, &chunk ), "push should have worked!" );

            subpath = string_advance_by( subpath, sep + 1 );
        } else {
            expect( darray_try_push( res, &subpath ), "push should have worked!" );
            break;
        }
    }

    return res;
}
b32 path_matches_glob( String path, String glob ) {
    if( glob.len == 1 && glob.cc[0] == '*' ) {
        return true;
    }

    while( path.len && *glob.cc != '*' ) {
        if( *glob.cc != *path.cc && *glob.cc != '?' ) {
            return false;
        }
        glob = string_advance( glob );
        path = string_advance( path );
    }

    String mp, cp;
    while( path.len ) {
        if( *glob.cc == '*' ) {
            glob = string_advance( glob );
            if( !glob.len ) {
                return true;
            }

            mp = glob;
            cp = string_advance( path );
        } else if( *glob.cc == *path.cc || *glob.cc == '?' ) {
            glob = string_advance( glob );
            path = string_advance( path );
        } else {
            glob = mp;
            cp   = string_advance( cp );
            path = cp;
        }
    }

    while( glob.len && *glob.cc == '*' ) {
        glob = string_advance( glob );
    }
    return glob.len ? false : true;

}
static b32 path_walk_glob_filter_filter(
    usize index, usize stride, const void* item, void* params
) {
    unused(index, stride);

    String glob = *(String*)params;
    String path = *(String*)item;

    return path_matches_glob( path, glob );
}
String* path_walk_glob_filter( const WalkDirectory* wd, String glob ) {
    assertion( wd && wd->paths, "walk result is null!" );

    String* res = darray_from_filter(
        sizeof(String), wd->count, wd->paths,
        path_walk_glob_filter_filter, &glob );
    return res;
}
b32 path_walk_dir(
    const char* dir, b32 recursive,
    b32 include_dirs, WalkDirectory* out_result
) {
    assertion( dir, "no path provided!" );
    assertion( out_result, "no walk dir result provided!" );

    DString* path = dstring_from_cstr( dir );
    if( !path ) {
        return false;
    }

    DString* buffer = dstring_empty( 255 );
    if( !buffer ) {
        dstring_free( path );
        cb_error( "path_walk_dir: failed to allocate buffer!" );
        return false;
    }

    usize count = 0;
    b32 result = path_walk_dir_internal(
        &path, recursive, include_dirs, &count, &buffer );
    dstring_free( path );

    if( !result ) {
        dstring_free( buffer );
        return false;
    }

    if( !count ) {
        dstring_free( buffer );
        return true;
    }

    usize total = out_result->count + count;
    String* paths = darray_empty( sizeof(String), total );
    if( !paths ) {
        dstring_free( buffer );
        return false;
    }

    if( out_result->buf ) {
        DString* concat =
            dstring_append( out_result->buf, string_from_dstring( buffer ) );
        dstring_free( buffer );
        if( !concat ) {
            return false;
        }
    } else {
        out_result->buf = buffer;
    }

    out_result->count = total;

    String rem = string_from_dstring( out_result->buf );
    while( rem.len ) {
        usize nul = 0;
        if( string_find( rem, 0, &nul ) ) {
            String current = rem;
            current.len    = nul;

            expect( darray_try_push( paths, &current ), "miscalculated path count!" );

            rem = string_advance_by( rem, nul + 1 );
        } else {
            expect( darray_try_push( paths, &rem ), "miscalculated path count!" );
            break;
        }
    }

    if( out_result->paths ) {
        darray_free( out_result->paths );
    }

    out_result->paths = paths;
    return true;
}
void path_walk_free( WalkDirectory* wd ) {
    if( wd ) {
        if( wd->paths ) {
            darray_free( wd->paths );
        }
        if( wd->buf ) {
            dstring_free( wd->buf );
        }

        memory_zero( wd, sizeof(*wd) );
    }
}
b32 dir_remove( const char* path, b32 recursive ) {
    if( !recursive ) {
        return dir_remove_internal( path );
    }

    WalkDirectory wd;
    memory_zero( &wd, sizeof(wd) );

    if( !path_walk_dir( path, true, false, &wd ) ) {
        cb_error( "dir_remove: failed to walk directory '%s'!", path );
        return false;
    }

    for( usize i = 0; i < wd.count; ++i ) {
        if( !file_remove( wd.paths[i].cc ) ) {
            cb_error( "dir_remove: failed to remove file '%s'!", wd.paths[i].cc );

            path_walk_free( &wd );
            return false;
        }
    }

    path_walk_free( &wd );

    if( !path_walk_dir( path, true, true, &wd ) ) {
        cb_error( "dir_remove: failed to walk directory '%s'!", path );
        return false;
    }

    for( usize i = 0; i < wd.count; ++i ) {
        if( !dir_remove_internal( wd.paths[i].cc ) ) {
            cb_error( "dir_remove: failed to remove dir '%s'!", wd.paths[i].cc );

            path_walk_free( &wd );
            return false;
        }
    }

    path_walk_free( &wd );
    return dir_remove_internal( path );
}

b32 fd_write_fmt_va( FD* file, const char* format, va_list va ) {
    char* formatted = local_fmt_va( format, va );
    return fd_write( file, strlen( formatted ), formatted, 0 );
}
b32 fd_write_fmt( FD* file, const char* format, ... ) {
    va_list va;
    va_start( va, format );
    b32 res = fd_write_fmt_va( file, format, va );
    va_end( va );
    return res;
}

void* darray_empty( usize stride, usize cap ) {
    struct DynamicArray* res = memory_alloc( sizeof(*res) + (stride * cap) );
    if( !res ) {
        return NULL;
    }
    res->stride = stride;
    res->cap    = cap;
    return res->buf;
}
void* darray_from_array( usize stride, usize len, const void* buf ) {
    struct DynamicArray* res = darray_head( darray_empty( stride, len + 2 ) );
    if( !res ) {
        return NULL;
    }

    memory_copy( res->buf, buf, len * stride );
    res->len = len;

    return res->buf;
}
usize darray_static_memory_requirement( usize stride, usize cap ) {
    return (stride * cap) + sizeof(struct DynamicArray);
}
void* darray_static( usize stride, usize cap, void* buf ) {
    struct DynamicArray* res = buf;
    res->stride = stride;
    res->cap    = cap;
    res->len    = 0;
    return res->buf;
}
void* darray_join( usize stride,
    usize lhs_len, const void* lhs, usize rhs_len, const void* rhs
) {
    struct DynamicArray* res =
        darray_head( darray_empty( stride, lhs_len + rhs_len + 2 ) );
    if( !res ) {
        return NULL;
    }

    memory_copy( res->buf, lhs, stride * lhs_len );
    memory_copy( res->buf + (stride * lhs_len), rhs, rhs_len );
    res->len = lhs_len + rhs_len;

    return res;
}
void* darray_from_filter(
    usize stride, usize len, const void* src,
    DarrayFilterFN* filter, void* filter_params
) {
    const u8* src_bytes = src;

    void* res = darray_empty( stride, 10 );
    if( !res ) {
        return NULL;
    }

    for( usize i = 0; i < len; ++i ) {
        const u8* item = src_bytes + (stride * i);
        if( filter( i, stride, item, filter_params ) ) {
            void* _new = darray_push( res, item );
            if( !_new ) {
                darray_free( res );
                return NULL;
            }
        }
    }

    return res;
}

void* darray_grow( void* darray, usize amount ) {
    struct DynamicArray* res = darray_head( darray );
    usize old_size = (res->stride * res->cap) + sizeof(*res);
    usize new_size = (res->stride * amount) + old_size;

    res = memory_realloc( res, old_size, new_size );
    if( !res ) {
        return NULL;
    }

    res->cap += amount;
    return res->buf;
}
void* darray_clone( const void* darray ) {
    return darray_from_array(
        darray_stride( darray ), darray_len( darray ), darray );
}
void darray_clear( void* darray ) {
    struct DynamicArray* head = darray_head( darray );
    memory_zero( head->buf, head->len * head->stride );
    head->len = 0;
}
void* darray_set_len( void* darray, usize len ) {
    if( len > darray_cap( darray ) ) {
        usize diff = len - darray_cap( darray );

        struct DynamicArray* head = darray_head( darray_grow( darray, diff ) );
        if( !head ) {
            return NULL;
        }

        head->len = len;
        return head->buf;
    } else if( len > darray_len( darray ) ) {
        struct DynamicArray* head = darray_head( darray );
        usize diff = head->len - len;
        memory_zero( head->buf + len, head->stride * diff );
        head->len = diff;

        return head->buf;
    } else {
        darray_truncate( darray, len );
        return darray;
    }
}
void darray_truncate( void* darray, usize max ) {
    struct DynamicArray* head = darray_head( darray );
    if( max >= head->len ) {
        return;
    }
    usize diff = head->len - max;
    memory_zero( head->buf + max, head->stride * diff );
    head->len = max;
}
void darray_trim( void* darray, usize amount ) {
    struct DynamicArray* head = darray_head( darray );
    darray_truncate( head->buf, amount > head->len ? 0 : head->len - amount );
}
b32 darray_try_push( void* darray, const void* item ) {
    struct DynamicArray* head = darray_head( darray );
    if( head->len == head->cap ) {
        return false;
    }

    memory_copy( head->buf + (head->stride * head->len), item, head->stride );
    head->len++;

    return true;
}
b32 darray_try_emplace( void* darray, const void* item, usize at ) {
    struct DynamicArray* head = darray_head( darray );
    if( head->len == head->cap ) {
        return false;
    }
    if( at >= head->len ) {
        cb_warn(
            "darray_emplace: attempted to emplace past darray bounds! "
            "len: %zu index: %zu", head->len, at );
        return false;
    }

    memory_move(
        head->buf + ( head->stride * ( at + 1 ) ),
        head->buf + ( head->stride * at ),
        head->stride * ( ( head->len + 1 ) - at ) );
    memory_copy( head->buf + ( head->stride * at ), item, head->stride );
    head->len++;

    return true;
}
b32 darray_try_insert( void* darray, usize count, const void* items, usize at ) {
    struct DynamicArray* head = darray_head( darray );
    if( head->len + count > head->cap ) {
        return false;
    }
    if( at >= head->len ) {
        cb_warn(
            "darray_insert: attempted to insert past darray bounds! "
            "len: %zu index: %zu", head->len, at );
        return false;
    }

    memory_move(
        head->buf + ( head->stride * ( at + count ) ),
        head->buf + ( head->stride * at ),
        head->stride * ( ( head->len + 1 ) - at ) );
    memory_copy( head->buf + ( head->stride * at ), items, head->stride * count );
    head->len += count;

    return true;
}
b32 darray_try_append( void* darray, usize count, const void* items ) {
    struct DynamicArray* head = darray_head( darray );
    if( head->len + count > head->cap ) {
        return false;
    }

    memory_copy(
        head->buf + (head->stride * head->len),
        items, head->stride * count );
    head->len += count;

    return true;
}
b32 darray_pop( void* darray, void* opt_out_item ) {
    struct DynamicArray* head = darray_head( darray );
    if( !head->len ) {
        return false;
    }

    head->len--;
    if( opt_out_item ) {
        memory_copy(
            opt_out_item, head->buf + (head->stride * head->len), head->stride );
    }

    memory_zero( head->buf + (head->stride * head->len), head->stride );
    return true;
}
void* darray_push( void* darray, const void* item ) {
    void* res = darray;
    if( darray_try_push( res, item ) ) {
        return res;
    }

    res = darray_grow( res, 5 );
    if( !res ) {
        return NULL;
    }

    darray_try_push( res, item );
    return res;
}
void* darray_emplace( void* darray, const void* item, usize at ) {
    if( at >= darray_len( darray ) ) {
        cb_warn(
            "darray_emplace: attempted to emplace past darray bounds! "
            "len: %zu index: %zu", darray_len( darray ), at );
        return NULL;
    }

    void* res = darray;
    if( darray_try_emplace( res, item, at ) ) {
        return res;
    }

    res = darray_grow( res, 5 );
    if( !res ) {
        return NULL;
    }

    darray_try_emplace( res, item, at );
    return res;
}
void* darray_insert( void* darray, usize count, const void* items, usize at ) {
    struct DynamicArray* res = darray_head( darray );

    if( at >= res->len ) {
        cb_warn(
            "darray_insert: attempted to insert past darray bounds! "
            "len: %zu index: %zu", res->len, at );
        return false;
    }

    if( darray_try_insert( res->buf, count, items, at ) ) {
        return res->buf;
    }

    res = darray_head( darray_grow( res->buf, (count - (res->cap - res->len)) + 5 ) );
    if( !res ) {
        return NULL;
    }

    darray_try_insert( res->buf, count, items, at );
    return res->buf;
}
void* darray_append( void* darray, usize count, const void* items ) {
    struct DynamicArray* res = darray_head( darray );
    if( darray_try_append( res->buf, count, items ) ) {
        return res;
    }

    res = darray_head( darray_grow( res->buf, (count - (res->cap - res->len)) + 5 ) );
    if( !res ) {
        return NULL;
    }

    darray_try_append( res->buf, count, items );
    return res->buf;
}
b32 darray_remove( void* darray, usize index ) {

    struct DynamicArray* head = darray_head( darray );
    if( !head->len || index > head->len ) {
        cb_warn(
            "darray_remove: attempted to remove past array bounds! "
            "len: %zu index: %zu", head->len, index );
        return false;
    }
    if( index == (head->len - 1) ) {
        return darray_pop( darray, 0 );
    }

    void* item_to_remove = head->buf + (head->stride * index);
    void* item_next      = (u8*)item_to_remove + head->stride;
    usize move_size      = (head->buf + (head->stride * head->cap)) - (u8*)item_next;

    memory_move( item_to_remove, item_next, move_size );
    head->len--;
    memory_zero( head->buf + (head->stride * head->len), head->stride );

    return true;
}
b32 darray_remove_range( void* darray, usize from_inclusive, usize to_exclusive ) {
    assertion( from_inclusive < to_exclusive,
        "darray_remove_range: invalid range provided! (%zu, %zu]",
        from_inclusive, to_exclusive );
    struct DynamicArray* head = darray_head( darray );
    if( !head->len || from_inclusive >= head->len || to_exclusive > head->len ) {
        cb_warn(
            "darray_remove_range: attempted to remove past array bounds! "
            "len: %zu range: (%zu, %zu]", head->len, from_inclusive, to_exclusive );
        return false;
    }

    // TODO(alicia): reimplement this properly
    for( usize i = from_inclusive; i < to_exclusive; ++i ) {
        darray_remove( darray, from_inclusive );
    }

    return true;
}
usize darray_remaining( const void* darray ) {
    return darray_cap( darray ) - darray_len( darray );
}
usize darray_len( const void* darray ) {
    return ((struct DynamicArray*)darray - 1)->len;
}
usize darray_cap( const void* darray ) {
    return ((struct DynamicArray*)darray - 1)->cap;
}
usize darray_stride( const void* darray ) {
    return ((struct DynamicArray*)darray - 1)->stride;
}
usize darray_total_size( const void* darray ) {
    return
        sizeof(struct DynamicArray) +
        (darray_cap( darray ) * darray_stride( darray ));
}
b32 darray_is_empty( const void* darray ) {
    return darray_len( darray ) == 0;
}
b32 darray_is_full( const void* darray ) {
    return darray_len( darray ) == darray_cap( darray );
}
void* darray_head( void* darray ) {
    if( !darray ) {
        return NULL;
    }
    return (struct DynamicArray*)darray - 1;
}
const void* darray_head_const( const void* darray ) {
    if( !darray ) {
        return NULL;
    }
    return (const struct DynamicArray*)darray - 1;
}
void darray_free( void* darray ) {
    if( darray ) {
        struct DynamicArray* head = darray_head( darray );
        usize total_size = (head->cap * head->stride) + sizeof(*head);
        memory_free( head, total_size );
    }
}

b32 job_enqueue( JobFN* job, void* params ) {
    volatile struct JobQueue* queue = get_job_queue();

    if( queue->pending >= CBUILD_MAX_JOBS ) {
        cb_warn(
            "attempted to enqueue job while queue is full!" );
        return false;
    }

    struct JobEntry entry;
    entry.proc   = job;
    entry.params = params;

    fence();

    atom back = atomic_add( &queue->back, 1 );
    queue->entries[back % CBUILD_MAX_JOBS] = entry;

    fence();

    atomic_add( &queue->len, 1 );
    atomic_add( &queue->pending, 1 );

    semaphore_signal( (Semaphore*)&queue->wakeup );
    return true;
}
b32 job_enqueue_timed( JobFN* job, void* params, u32 ms ) {
    volatile struct JobQueue* queue = get_job_queue();

    while( queue->pending >= CBUILD_MAX_JOBS ) {
        if( !job_wait_next( ms ) ) {
            return false;
        }
    }

    expect( job_enqueue( job, params ),
        "enqueue unexpectedly failed!" );
    return true;
}
b32 job_wait_next( u32 ms ) {
    volatile struct JobQueue* queue = get_job_queue();

    u32 current = queue->pending;
    if( !current ) {
        return true;
    }

    if( ms == MT_WAIT_INFINITE ) {
        while( queue->pending >= current ) {
            thread_sleep( 1 );
        }
        return true;
    } else for( u32 i = 0; i < ms; ++i ) {
        if( queue->pending < current ) {
            return true;
        }
        thread_sleep( 1 );
    }

    return false;
}
b32 job_wait_all( u32 ms ) {
    volatile struct JobQueue* queue = get_job_queue();

    if( ms == MT_WAIT_INFINITE ) {
        while( queue->pending ) {
            thread_sleep(1);
        }
        return true;
    } else for( u32 i = 0; i < ms; ++i ) {
        if( !queue->pending ) {
            return true;
        }
        thread_sleep( 1 );
    }

    return false;
}

u32 thread_id(void) {
    return global_thread_id;
}
b32 command_builder_new( const char* exe, CommandBuilder* out_builder ) {
    out_builder->buf = darray_empty( sizeof(char), 128 );
    if( !out_builder->buf ) {
        return false;
    }
    out_builder->args = darray_empty( sizeof(const char*), 5 );
    if( !out_builder->args ) {
        darray_free( out_builder->buf );
        return false;
    }
    command_builder_push( out_builder, exe );
    return true;
}
void command_builder_clear( CommandBuilder* builder ) {
    darray_clear( builder->buf );
    darray_clear( builder->args );
}
b32 command_builder_push( CommandBuilder* builder, const char* arg ) {
    usize arg_len = strlen( arg );
    if( darray_remaining( builder->buf ) < arg_len + 1 ) {
        Darray(char) new_buf = darray_grow( builder->buf, darray_cap( builder->buf ) );
        if( !new_buf ) {
            return false;
        }
        builder->buf = new_buf;
    }
    if( darray_remaining( builder->args ) < 1 ) {
        Darray(const char*) new_buf =
            darray_grow( builder->args, darray_cap( builder->args ) );
        if( !new_buf ) {
            return false;
        }
        builder->args = new_buf;
    }
    const char* new_arg = builder->buf + darray_len( builder->buf );
    darray_try_append( builder->buf, arg_len + 1, arg );

    darray_pop( builder->args, 0 );
    darray_try_push( builder->args, &new_arg );
    const char* null = 0;
    darray_try_push( builder->args, &null );

    return true;
}
b32 __internal_command_builder_append(
    CommandBuilder* builder, const char* first, ...
) {
    if( !command_builder_push( builder, first ) ) {
        return false;
    }
    va_list va;
    va_start( va, first );

    for( ;; ) {
        const char* next = va_arg( va, const char* );
        if( !next ) {
            break;
        }

        if( !command_builder_push( builder, next ) ) {
            va_end(va);
            return false;
        }
    }

    va_end(va);
    return true;
}
b32 command_builder_append_list(
    CommandBuilder* builder, usize count, const char** args
) {
    for( usize i = 0; i < count; ++i ) {
        if( !command_builder_push( builder, args[i])) {
            return false;
        }
    }
    return true;
}
Command command_builder_cmd( CommandBuilder* builder ) {
    return (Command){
        .count = darray_len(builder->args) - 1,
        .args  = builder->args,
    };
}
void command_builder_free( CommandBuilder* builder ) {
    darray_free( builder->buf );
    darray_free( builder->args );
    memory_zero( builder, sizeof(*builder) );
}
DString* command_flatten_dstring( const Command* command ) {
    usize total_len = 1;
    for( usize i = 0; i < command->count; ++i ) {
        total_len += strlen( command->args[i] ) + 3; // to account for potential "" and space
    }
    DString* res = dstring_empty( total_len );
    if( !res ) {
        return NULL;
    }

    for( usize i = 0; i < command->count; ++i ) {
        const char* current = command->args[i];
        if( !current ) {
            continue;
        }

        usize current_len = strlen( current );
        if( !current_len ) {
            continue;
        }

        b32 contains_space = false;
        String arg = string_new( current_len, current );
        if( string_find( arg, ' ', 0 ) ) {
            expect( dstring_push( res, '"' ) == res, "miscalculated total_len!" );
            contains_space = true;
        }

        expect( dstring_append( res, arg ) == res, "miscalculated total_len!" );

        if( contains_space ) {
            expect( dstring_push( res, '"' ) == res, "miscalculated total_len!" );
        }

        if( i + 1 != command->count ) {
            expect( dstring_push( res, ' ' ) == res, "miscalculated total_len!" );
        }
    }

    return res;
}

u8* local_byte_buffer() {
    fence();
    return (u8*)get_next_local_buffer( thread_id() );
}
char* local_fmt_va( const char* format, va_list va ) {
    char* buf = (char*)local_byte_buffer();
    vsnprintf( buf, CBUILD_LOCAL_BUFFER_CAPACITY - 1, format, va ); // -1 to ensure null-terminated
    return buf;
}
char* local_fmt( const char* format, ... ) {
    va_list va;
    va_start( va, format );
    char* res = local_fmt_va( format, va );
    va_end( va );
    return res;
}

static b32 logger_check_level( LoggerLevel level ) {
    return level >= global_logger_level;
}

void logger_set_level( LoggerLevel level ) {
    global_logger_level = level;
}
LoggerLevel logger_get_level(void) {
    return global_logger_level;
}
void logger_va( LoggerLevel level, const char* format, va_list va ) {
    if( !logger_check_level( level ) ) {
        return;
    }

    static const char local_level_letters[] = {
        'I', // LOGGER_LEVEL_INFO
        'W', // LOGGER_LEVEL_WARNING
        'E', // LOGGER_LEVEL_ERROR
        'F', // LOGGER_LEVEL_FATAL
    };

    static const char* local_level_colors[] = {
        "",           // LOGGER_LEVEL_INFO
        "\033[1;33m", // LOGGER_LEVEL_WARNING
        "\033[1;31m", // LOGGER_LEVEL_ERROR
        "\033[1;35m", // LOGGER_LEVEL_FATAL
    };

    FILE* const level_output[] = {
        stdout, // LOGGER_LEVEL_INFO
        stdout, // LOGGER_LEVEL_WARNING
        stderr, // LOGGER_LEVEL_ERROR
        stderr, // LOGGER_LEVEL_FATAL
    };

    if( global_is_mt ) {
        mutex_lock( &global_logger_mutex );
    }

    FILE* output = level_output[level];

    fprintf( output, "%s[%c:%02u] cbuild: ",
        local_level_colors[level], local_level_letters[level], thread_id() );
    vfprintf( output, format, va );
    fprintf( output, "\033[1;00m\n" );

    fflush( output );

    if( global_is_mt ) {
        mutex_unlock( &global_logger_mutex );
    }
}
void logger( LoggerLevel level, const char* format, ... ) {
    va_list va;
    va_start( va, format );
    logger_va( level, format, va );
    va_end( va );
}

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <limits.h>

#if !defined(PATH_MAX)
    #define PATH_MAX 260
#endif

struct Win32ThreadParams {
    JobFN* proc;
    void*  params;
    u32    id;
};

#define CBUILD_LOCAL_WIDE_CAPACITY (CBUILD_LOCAL_BUFFER_CAPACITY / 2)

static struct Win32ThreadParams global_win32_thread_params[CBUILD_THREAD_COUNT];

static HANDLE global_win32_process_heap = NULL;

void _platform_init_(void) {
    SetConsoleCP( CP_UTF8 );
    SetConsoleOutputCP( CP_UTF8 );
}
static HANDLE get_process_heap(void) {
    if( !global_win32_process_heap ) {
        global_win32_process_heap = GetProcessHeap();
        expect( global_win32_process_heap, "failed to get process heap!" );
    }
    return global_win32_process_heap;
}
static time_t win32_filetime_to_posix( FILETIME ft ) {
    #define WIN32_TICKS_PER_SECOND (10000000)
    #define WIN32_TO_POSIX_DIFF    (11644473600ULL)

    ULARGE_INTEGER uli;
    uli.LowPart  = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    time_t res = (time_t)(
        (uli.QuadPart / WIN32_TICKS_PER_SECOND) - WIN32_TO_POSIX_DIFF );

    #undef WIN32_TICKS_PER_SECOND
    #undef WIN32_TO_POSIX_DIFF
    
    return res;
}
static wchar_t* win32_local_utf8_to_wide( String utf8 ) {
    assertion( utf8.cc, "null pointer!" );
    assertion(
        utf8.len < CBUILD_LOCAL_WIDE_CAPACITY,
        "attempted to convert a utf8 string longer than %u!",
        CBUILD_LOCAL_WIDE_CAPACITY );

    wchar_t* buf = (wchar_t*)local_byte_buffer();
    wchar_t* dst = buf;

    const char* src = utf8.cc;
    usize rem       = utf8.len;

    while( rem ) {
        usize max_convert = rem;
        if( max_convert >= INT32_MAX ) {
            max_convert = INT32_MAX;
        }
        // TODO(alicia): utf8 bounds checking!

        int written = MultiByteToWideChar(
            CP_UTF8, 0, src, max_convert, dst, max_convert );
        expect( written, "failed to convert utf8 string!" );

        src += max_convert;
        dst += written;
        rem -= max_convert;
    }

    return buf;
}
static String win32_local_wide_to_utf8( usize len, wchar_t* wide ) {
    assertion( wide, "null pointer!" );
    assertion(
        len < CBUILD_LOCAL_WIDE_CAPACITY,
        "attempted to convert a wide string longer than %u!",
        CBUILD_LOCAL_WIDE_CAPACITY );

    char* buf = (char*)local_byte_buffer();
    char* dst = buf;

    wchar_t* src = wide;
    usize rem    = len;
    while( rem ) {
        usize max_convert = rem;
        if( max_convert >= INT32_MAX ) {
            max_convert = INT32_MAX;
        }
        // bounds checking
        const char* last_codepoint = (const char*)(src + (max_convert - 1));
        if( last_codepoint[0] & 0b10000000 ) {
            max_convert--;
            assertion( max_convert, "invalid wide string!" );
        }
        
        int written = WideCharToMultiByte(
            CP_UTF8, 0, src, max_convert, dst, max_convert, 0, 0 );
        expect( written, "failed to convert wide string!" );

        src += max_convert;
        dst += written;
        rem -= max_convert;
    }

    return string_new( dst - buf, buf );
}
static String win32_local_error_message( DWORD error_code ) {
    char* buf = (char*)local_byte_buffer();

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM, 0, error_code,
        0, buf, CBUILD_LOCAL_BUFFER_CAPACITY, 0 );

    return string_from_cstr( buf );
}
static wchar_t* win32_local_path_canon( String path ) {
    #define PATH_RELATIVE 0
    #define PATH_HOME     1
    #define PATH_ABSOLUTE 2

    int path_type = PATH_RELATIVE;

    if( path.len >= sizeof("A:")  ) {
        if( isalpha( path.cc[0] ) && path.cc[1] == ':' ) {
            path_type = PATH_ABSOLUTE;
        }
    } else if( path.cc[0] == '~' ) {
        path_type = PATH_HOME;
    }

    String subpath = path;

    size_t min    = sizeof("\\\\?\\A:");
    size_t offset = sizeof("\\\\?\\") - 1;

    const char* home       = NULL;
    const char* home_drive = NULL;
    size_t home_drive_len  = 0;
    size_t home_len        = 0;
    size_t pwd_len         = 0;

    switch( path_type ) {
        case PATH_RELATIVE:  {
            pwd_len   = GetCurrentDirectoryW( 0, 0 );
        } break;
        case PATH_ABSOLUTE: break;
        case PATH_HOME: {
            home_drive = getenv( "HOMEDRIVE" );
            home       = getenv( "HOMEPATH" );

            home_drive_len = strlen( home_drive );
            home_len       = strlen( home );
        } break;
        default: break;
    }

    wchar_t* buf = (wchar_t*)local_byte_buffer();
    memory_copy( buf, L"\\\\?\\", sizeof(L"\\\\?\\") - sizeof(wchar_t) );
    switch( path_type ) {
        case PATH_RELATIVE:  {
            offset += GetCurrentDirectoryW(
                CBUILD_LOCAL_WIDE_CAPACITY - offset, buf + offset );
        } break;
        case PATH_ABSOLUTE: break;
        case PATH_HOME: {
            MultiByteToWideChar(
                CP_UTF8, 0, home_drive, home_drive_len,
                buf + offset, CBUILD_LOCAL_WIDE_CAPACITY - offset );
            offset += home_drive_len;
            MultiByteToWideChar(
                CP_UTF8, 0, home, home_len,
                buf + offset, CBUILD_LOCAL_WIDE_CAPACITY - offset );
            offset += home_len;

            subpath = string_advance( subpath );
        } break;
        default: break;
    }

    size_t last_chunk_len = 0;
    while( subpath.len ) {
        size_t sep = 0;
        if( string_find( subpath, '/', &sep ) ) {
            if( !sep ) {
                subpath = string_advance( subpath );
                continue;
            }

            String chunk = subpath;
            chunk.len    = sep;

            if( chunk.len < 3 ) {
                if( string_cmp( chunk, string_text( "." ) ) ) {
                    subpath = string_advance_by( subpath, chunk.len + 1 );
                    continue;
                }
                if( string_cmp( chunk, string_text( ".." ) ) ) {
                    for( size_t i = offset; i-- > 0; ) {
                        wchar_t c = buf[i];
                        if( c == '\\' ) {
                            offset = i;
                            break;
                        }
                    }
                    if( offset < min ) {
                        offset = min;
                    }
                    buf[offset] = 0;
                    subpath = string_advance_by( subpath, chunk.len + 1 );
                    continue;
                }
            }

            if( buf[offset - 1] != '\\' ) {
                buf[offset++] = '\\';
            }
            if( offset + chunk.len >= CBUILD_LOCAL_WIDE_CAPACITY ) {
                break;
            }
            MultiByteToWideChar(
                CP_UTF8, 0, chunk.cc, chunk.len,
                buf + offset, CBUILD_LOCAL_WIDE_CAPACITY - offset );
            offset += chunk.len;
            last_chunk_len = chunk.len;

            subpath = string_advance_by( subpath, chunk.len + 1 );
        } else {
            if( string_cmp( subpath, string_text( "." ) ) ) {
                break;
            }
            if( string_cmp( subpath, string_text( ".." ) ) ) {
                for( size_t i = offset; i-- > 0; ) {
                    wchar_t c = buf[i];
                    if( c == '\\' ) {
                        offset = i;
                        break;
                    }
                }
                if( offset < min ) {
                    offset = min;
                }
                buf[offset] = 0;
                break;
            }

            if( buf[offset - 1] != '\\' ) {
                buf[offset++] = '\\';
            }
            if( offset + subpath.len >= CBUILD_LOCAL_WIDE_CAPACITY ) {
                break;
            }
            MultiByteToWideChar(
                CP_UTF8, 0, subpath.cc, subpath.len,
                buf + offset, CBUILD_LOCAL_WIDE_CAPACITY - offset );
            offset += subpath.len;
            break;
        }
    }
    buf[offset] = 0;

    #undef PATH_RELATIVE
    #undef PATH_HOME    
    #undef PATH_ABSOLUTE
    return buf;
}

void* memory_alloc( usize size ) {
    void* res = HeapAlloc( get_process_heap(), HEAP_ZERO_MEMORY, size );
    if( !res ) {
        return NULL;
    }
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, size );
        atomic_add64( &global_total_memory_usage, size );
    } else {
        global_memory_usage       += size;
        global_total_memory_usage += size;
    }
    return res;
}
void* memory_realloc( void* memory, usize old_size, usize new_size ) {
    assertion( new_size >= old_size, "attempted to reallocate to smaller buffer!" );
    void* res = HeapReAlloc( get_process_heap(), HEAP_ZERO_MEMORY, memory, new_size );
    if( !res ) {
        return NULL;
    }
    usize diff = new_size - old_size;
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, diff );
        atomic_add64( &global_total_memory_usage, diff );
    } else {
        global_memory_usage       += diff;
        global_total_memory_usage += diff;
    }
    return res;
}
void memory_free( void* memory, usize size ) {
    if( !memory ) {
        cb_warn( "attempted to free null pointer!" );
        return;
    }
    HeapFree( get_process_heap(), 0, memory );
    atom64 neg = size;
    neg = -neg;
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, neg );
    } else {
        global_memory_usage += neg;
    }
}
b32 path_is_absolute( const char* path ) {
    assertion( path, "null path!" );
    if( !path[0] || !path[1] ) {
        return false;
    }
    return
        isalpha( path[0] ) &&
        path[1] == ':';
}
static DWORD path_attributes( String path ) {
    wchar_t* wpath = win32_local_path_canon( path );
    return GetFileAttributesW( wpath );
}
b32 path_is_directory( const char* path ) {
    String spath = string_from_cstr( path );
    DWORD attr   = path_attributes( spath );

    if( attr == INVALID_FILE_ATTRIBUTES ) {
        return false;
    } else {
        return attr & FILE_ATTRIBUTE_DIRECTORY;
    }
}
b32 path_exists( const char* path ) {
    assertion( path, "null path!" );
    String spath = string_from_cstr( path );
    return path_attributes( spath ) != INVALID_FILE_ATTRIBUTES;
}
static b32 path_walk_dir_internal_long(
    DString** path, b32 recursive, b32 include_dirs,
    usize* out_count, DString** out_buffer
) {
    usize original_len = dstring_len( *path );
    DString* _new = dstring_append( *path, string_text( "/*" ) );
    if( !_new ) {
        return false;
    }
    *path = _new;

    wchar_t* wpath = win32_local_path_canon( string_from_dstring( *path ) );

    WIN32_FIND_DATAW fd;
    memory_zero( &fd, sizeof( fd ) );
    HANDLE find_file = FindFirstFileW( wpath, &fd );
    dstring_truncate( *path, original_len );

    if( find_file == INVALID_HANDLE_VALUE ) {
        return false;
    }

    do {
        if(
            wcscmp( fd.cFileName, L"." )    == 0 ||
            wcscmp( fd.cFileName, L".." )   == 0 ||
            wcscmp( fd.cFileName, L".git" ) == 0
        ) {
            continue;
        }

        _new = dstring_push( *path, '/' );
        if( !_new ) {
            FindClose( find_file );
            return false;
        }
        *path = _new;

        usize name_len = wcslen( fd.cFileName );
        String npath   = win32_local_wide_to_utf8( name_len, fd.cFileName );

        _new = dstring_append( *path, npath );
        if( !_new ) {
            FindClose( find_file );
            return false;
        }
        *path = _new;

        if( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
            if( include_dirs ) {
                _new = dstring_append(
                    *out_buffer, string_new( dstring_len( *path ) + 1 , *path ) );
                if( !_new ) {
                    FindClose( find_file );
                    return false;
                }
                *out_buffer = _new;
                *out_count += 1;
            }

            if( recursive ) {
                if( !path_walk_dir_internal_long(
                    path, recursive, include_dirs, out_count, out_buffer
                ) ) {
                    FindClose( find_file );
                    return false;
                }
            }

            dstring_truncate( *path, original_len );
            continue;
        }

        _new = dstring_append(
            *out_buffer, string_new( dstring_len( *path ) + 1, *path ) );
        if( !_new ) {
            FindClose( find_file );
            return false;
        }
        *out_buffer = _new;
        *out_count += 1;

        dstring_truncate( *path, original_len );
    } while( FindNextFileW( find_file, &fd ) != FALSE );

    FindClose( find_file );
    return true;
}
static b32 path_walk_dir_internal(
    DString** path, b32 recursive, b32 include_dirs,
    usize* out_count, DString** out_buffer
) {
    return path_walk_dir_internal_long(
        path, recursive, include_dirs, out_count, out_buffer );
}

static DWORD fd_open_dwaccess( FileOpenFlags flags ) {
    DWORD res = 0;
    if( flags & FOPEN_READ ) {
        res |= GENERIC_READ;
    }
    if( flags & FOPEN_WRITE ) {
        res |= GENERIC_WRITE;
    }
    return res;
}
static DWORD fd_open_dwcreationdisposition( FileOpenFlags flags ) {
    DWORD res = OPEN_EXISTING;
    if( flags & FOPEN_CREATE ) {
        res = CREATE_ALWAYS;
    } else if( flags & FOPEN_TRUNCATE ) {
        res = TRUNCATE_EXISTING;
    }
    return res;
}
static b32 fd_open_long( String path, FileOpenFlags flags, FD* out_file ) {
    DWORD dwDesiredAccess       = fd_open_dwaccess( flags );
    DWORD dwShareMode           = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD dwCreationDisposition = fd_open_dwcreationdisposition( flags );
    DWORD dwFlagsAndAttributes  = 0;

    wchar_t* wide = win32_local_path_canon( path );

    HANDLE handle = CreateFileW(
        wide, dwDesiredAccess, dwShareMode, 0,
        dwCreationDisposition, dwFlagsAndAttributes, 0 );
    if( handle == INVALID_HANDLE_VALUE ) {
        DWORD error_code = GetLastError();
        String msg = win32_local_error_message( error_code );

        cb_error(
            "failed to open '%S'! reason: (0x%X) %s", wide, error_code, msg.cc );
        return false;
    }

    *out_file = (usize)handle;
    return true;
}

b32 fd_open( const char* path, FileOpenFlags flags, FD* out_file ) {
    if( !validate_file_flags( flags ) ) {
        return false;
    }
    String path_str = string_from_cstr( path );
    return fd_open_long( path_str, flags, out_file );
}
void fd_close( FD* file ) {
    CloseHandle( (HANDLE)*file );
    *file = 0;
}

static b32 fd_write_32(
    FD* file, DWORD size, const void* buf, DWORD* opt_out_write_size
) {
    HANDLE hFile = (HANDLE)*file;
    b32 is_console = GetFileType( hFile ) == FILE_TYPE_CHAR;

    DWORD out_size = 0;
    BOOL res = FALSE;
    if( is_console ) {
        // NOTE(alicia): actually UTF-8 encoded
        // because of _platform_init_ SetConsoleOutputCP to CP_UTF8
        res = WriteConsoleA( hFile, buf, size, &out_size, 0 );
    } else {
        res = WriteFile( hFile, buf, size, &out_size, 0 );
    }
    if( opt_out_write_size ) {
        *opt_out_write_size = out_size;
    }
    return res;
}
#if defined(ARCH_64BIT)
static b32 fd_write_64(
    FD* file, usize size, const void* buf, usize* opt_out_write_size
) {
    DWORD size0 = size > UINT32_MAX ? UINT32_MAX : size;
    DWORD size1 = size > UINT32_MAX ? size - UINT32_MAX : 0;

    usize write_total = 0;
    DWORD write_size  = 0;
    b32 res = fd_write_32( file, size0, buf, &write_size );
    write_total = write_size;

    if( write_size != size0 ) {
        if( opt_out_write_size ) {
            *opt_out_write_size = write_total;
        }
        return res;
    }
    if( !res ) {
        return false;
    }

    if( size1 ) {
        res = fd_write_32( file, size1, (const u8*)buf + size0, &write_size );
        if( !res ) {
            return false;
        }
        write_total += write_size;
    }

    if( opt_out_write_size ) {
        *opt_out_write_size = write_total;
    }
    return true;
}
#endif

static b32 fd_read_32(
    FD* file, DWORD size, void* buf, DWORD* opt_out_read_size
) {
    DWORD out_size = 0;
    BOOL res = ReadFile( (HANDLE)*file, buf, size, &out_size, 0 );
    if( opt_out_read_size ) {
        *opt_out_read_size = out_size;
    }
    return res;
}
#if defined(ARCH_64BIT)
static b32 fd_read_64(
    FD* file, usize size, void* buf, usize* opt_out_read_size
) {
    DWORD size0 = size > UINT32_MAX ? UINT32_MAX : size;
    DWORD size1 = size > UINT32_MAX ? size - UINT32_MAX : 0;

    usize read_total = 0;
    DWORD read_size  = 0;
    b32 res = fd_read_32( file, size0, buf, &read_size );
    read_total = read_size;

    if( read_size != size0 ) {
        if( opt_out_read_size ) {
            *opt_out_read_size = read_total;
        }
        return res;
    }
    if( !res ) {
        return false;
    }

    if( size1 ) {
        res = fd_read_32( file, size1, (u8*)buf + size0, &read_size );
        if( !res ) {
            return false;
        }
        read_total += read_size;
    }

    if( opt_out_read_size ) {
        *opt_out_read_size = read_total;
    }
    return true;
}
#endif

b32 fd_write( FD* file, usize size, const void* buf, usize* opt_out_write_size ) {
#if defined(ARCH_64BIT)
    return fd_write_64( file, size, buf, opt_out_write_size );
#else
    return fd_write_32( file, size, buf, (DWORD*)opt_out_write_size );
#endif
}
b32 fd_read( FD* file, usize size, void* buf, usize* opt_out_read_size ) {
#if defined(ARCH_64BIT)
    return fd_read_64( file, size, buf, opt_out_read_size );
#else
    return fd_read_32( file, size, buf, (DWORD*)opt_out_read_size );
#endif
}
b32 fd_truncate( FD* file ) {
    return SetEndOfFile( (HANDLE)*file ) != FALSE;
}
usize fd_query_size( FD* file ) {
#if defined(ARCH_64BIT)
    LARGE_INTEGER li;
    expect( GetFileSizeEx( (HANDLE)*file, &li ), "failed to get file size!" );
    return li.QuadPart;
#else
    DWORD res = GetFileSize( (HANDLE)*file, 0 );
    return res;
#endif
}
void fd_seek( FD* file, FileSeek type, isize seek ) {
    DWORD dwMoveMethod = 0;
    switch( type ) {
        case FSEEK_CURRENT: {
            dwMoveMethod = FILE_CURRENT;
        } break;
        case FSEEK_BEGIN: {
            dwMoveMethod = FILE_BEGIN;
        } break;
        case FSEEK_END: {
            dwMoveMethod = FILE_END;
        } break;
    }

#if defined(ARCH_64BIT)
    LARGE_INTEGER li;
    li.QuadPart = seek;

    expect(
        SetFilePointerEx( (HANDLE)*file, li, 0, dwMoveMethod ) != FALSE,
        "failed to seek file!" );
#else
    SetFilePointer( (HANDLE)*file, seek, 0, dwMoveMethod );
#endif
}
usize fd_query_position( FD* file ) {
    DWORD dwMoveMethod = FILE_CURRENT;

#if defined(ARCH_64BIT)
    LARGE_INTEGER li;
    li.QuadPart = 0;

    LARGE_INTEGER res;
    expect(
        SetFilePointerEx( (HANDLE)*file, li, &res, dwMoveMethod ) != FALSE,
        "failed to query file position!" );

    return res.QuadPart;
#else
    DWORD res = SetFilePointer( (HANDLE)*file, 0, 0, dwMoveMethod );
    return res;
#endif

}
static void file_query_time_long(
    String path, FILETIME* out_create, FILETIME* out_modify
) {
    DWORD dwDesiredAccess       = 0;
    DWORD dwCreationDisposition = OPEN_EXISTING;
    DWORD dwFlagsAndAttributes  = 0;
    DWORD dwShareMode           =
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    wchar_t* wpath = win32_local_path_canon( path );

    HANDLE handle = CreateFileW(
        wpath, dwDesiredAccess, dwShareMode,
        NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL );

    expect( handle != INVALID_HANDLE_VALUE, "failed to open file!" );
    expect(
        GetFileTime( handle, out_create, 0, out_modify ),
        "failed to get file time!" );

    CloseHandle( handle );
}
time_t file_query_time_create( const char* path ) {
    FILETIME create;
    memory_zero( &create, sizeof(create) );

    usize path_len = strlen( path );
    file_query_time_long( string_new( path_len, path ), &create, 0 );

    return win32_filetime_to_posix( create );
}
time_t file_query_time_modify( const char* path ) {
    FILETIME modify;
    memory_zero( &modify, sizeof(modify) );

    usize path_len = strlen( path );
    file_query_time_long( string_new( path_len, path ), 0, &modify );
    return win32_filetime_to_posix( modify );
}

static b32 file_move_long( String dst, String src ) {
    wchar_t* dst_wide = win32_local_path_canon( dst );
    wchar_t* src_wide = win32_local_path_canon( src );

    b32 res = MoveFileW( src_wide, dst_wide );
    return res == TRUE;
}

b32 file_move( const char* dst, const char* src ) {
    assertion( dst && src, "null path provided!" );
    usize dst_len = strlen( dst );
    usize src_len = strlen( src );

    return
        file_move_long( string_new( dst_len, dst ), string_new( src_len, src ) );
}

static b32 file_copy_long( String dst, String src ) {
    wchar_t* dst_wide = win32_local_path_canon( dst );
    wchar_t* src_wide = win32_local_path_canon( src );

    b32 res = CopyFileW( src_wide, dst_wide, FALSE );
    return res == TRUE;
}

b32 file_copy( const char* dst, const char* src ) {
    assertion( dst && src, "null path provided!" );
    usize dst_len = strlen( dst );
    usize src_len = strlen( src );

    return
        file_copy_long( string_new( dst_len, dst ), string_new( src_len, src ) );
}
static b32 file_remove_long( String path ) {
    wchar_t* path_wide = win32_local_path_canon( path );
    return DeleteFileW( path_wide ) != FALSE;
}
b32 file_remove( const char* path ) {
    usize path_len = strlen( path );
    return file_remove_long( string_new( path_len, path ) );
}
static b32 dir_create_long( String path ) {
    wchar_t* wpath = win32_local_path_canon( path );
    if( CreateDirectoryW( wpath, 0 ) ) {
        return true;
    } else {
        DWORD res = GetLastError();
        return res == ERROR_ALREADY_EXISTS;
    }
}
b32 dir_create( const char* path ) {
    usize len = strlen( path );
    return dir_create_long( string_new( len, path ) );
}
static b32 dir_remove_internal_long( String path ) {
    wchar_t* wpath = win32_local_path_canon( path );
    return RemoveDirectoryW( wpath );
}
static b32 dir_remove_internal( const char* path ) {
    usize len = strlen( path );
    return dir_remove_internal_long( string_new( len, path ) );
}

atom atomic_add( atom* atomic, atom val ) {
    return _InterlockedExchangeAdd( atomic, val );
}
atom64 atomic_add64( atom64* atom, atom64 val ) {
    return _InterlockedExchangeAdd64( atom, val );
}
atom atomic_compare_swap( atom* atomic, atom comp, atom exch ) {
    return _InterlockedCompareExchange( atomic, exch, comp );
}
atom64 atomic_compare_swap64( atom64* atom, atom64 comp, atom64 exch ) {
    return _InterlockedCompareExchange64( atom, exch, comp );
}

void fence(void) {
    MemoryBarrier();
}

f64 timer_milliseconds(void) {
    return timer_seconds() * 1000.0;
}
f64 timer_seconds(void) {
    LARGE_INTEGER qpf, qpc;
    QueryPerformanceFrequency( &qpf );
    QueryPerformanceCounter( &qpc );

    return (f64)qpc.QuadPart / (f64)qpf.QuadPart;
}

b32 mutex_create( Mutex* out_mutex ) {
    HANDLE handle = CreateMutexA( 0, 0, 0 );
    if( !handle ) {
        return false;
    }
    out_mutex->handle = (void*)handle;
    return true;
}
b32 mutex_is_valid( const Mutex* mutex ) {
    return mutex->handle != NULL;
}
void mutex_lock( Mutex* mutex ) {
    WaitForSingleObject( mutex->handle, INFINITE );
}
b32 mutex_lock_timed( Mutex* mutex, u32 ms ) {
    if( ms == INFINITE ) {
        mutex_lock( mutex );
        return true;
    }
    DWORD  result = WaitForSingleObject( mutex->handle, ms );
    return result != WAIT_TIMEOUT;
}
void mutex_unlock( Mutex* mutex ) {
    ReleaseMutex( mutex->handle );
}
void mutex_destroy( Mutex* mutex ) {
    CloseHandle( mutex->handle );
    memory_zero( mutex, sizeof(*mutex) );
}

b32 semaphore_create( Semaphore* out_semaphore ) {
    HANDLE handle = CreateSemaphoreA( NULL, 0, INT32_MAX, 0 );
    if( !handle ) {
        return false;
    }
    out_semaphore->handle = handle;
    return true;
}
b32 semaphore_is_valid( const Semaphore* semaphore ) {
    return semaphore->handle != NULL;
}
void semaphore_wait( Semaphore* semaphore ) {
    WaitForSingleObject( semaphore->handle, INFINITE );
}
b32 semaphore_wait_timed( Semaphore* semaphore, u32 ms ) {
    if( ms == INFINITE ) {
        semaphore_wait( semaphore );
        return true;
    }
    DWORD  result = WaitForSingleObject( semaphore->handle, ms );
    return result != WAIT_TIMEOUT;
}
void semaphore_signal( Semaphore* semaphore ) {
    ReleaseSemaphore( semaphore->handle, 1, NULL );
}
void semaphore_destroy( Semaphore* semaphore ) {
    CloseHandle( semaphore->handle );
    memory_zero( semaphore, sizeof(*semaphore) );
}

void thread_sleep( u32 ms ) {
    Sleep( ms );
}

void pipe_open( ReadPipe* out_read, WritePipe* out_write ) {
    HANDLE read = 0, write = 0;
    SECURITY_ATTRIBUTES sa;
    memory_zero( &sa, sizeof(sa) );

    sa.nLength        = sizeof(sa);
    sa.bInheritHandle = TRUE;

    expect( CreatePipe( &read, &write, &sa, 0 ), "failed to create pipes!" );

    *out_read  = (isize)read;
    *out_write = (isize)write;
}
void pipe_close( Pipe pipe ) {
    HANDLE handle = (HANDLE)pipe;
    CloseHandle( handle );
}
b32 process_in_path( const char* process_name ) {
    char* cmd = local_fmt( "where.exe %s /Q", process_name );
    return system( cmd ) == 0;
}
PID process_exec(
    Command cmd, b32 redirect_void, ReadPipe* opt_stdin,
    WritePipe* opt_stdout, WritePipe* opt_stderr, const char* opt_cwd
) {
    STARTUPINFOW        startup;
    PROCESS_INFORMATION info;

    memory_zero( &startup, sizeof(startup) );
    memory_zero( &info,    sizeof(info) );

    startup.cb         = sizeof(startup);
    startup.hStdInput  = GetStdHandle( STD_INPUT_HANDLE );
    startup.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    startup.hStdError  = GetStdHandle( STD_ERROR_HANDLE );

    BOOL bInheritHandle = FALSE;
    if( redirect_void ) {
        bInheritHandle = TRUE;
        volatile struct GlobalBuffers* gb = get_global_buffers();
        startup.hStdInput  = (HANDLE)gb->void_read;
        startup.hStdOutput = (HANDLE)gb->void_write;
        startup.hStdError  = (HANDLE)gb->void_write;
    } else {
        if( opt_stdin ) {
            startup.hStdInput = (HANDLE)*opt_stdin;
            bInheritHandle    = TRUE;
        }
        if( opt_stdout ) {
            startup.hStdOutput = (HANDLE)*opt_stdout;
            bInheritHandle     = TRUE;
        }
        if( opt_stderr ) {
            startup.hStdError = (HANDLE)*opt_stderr;
            bInheritHandle    = TRUE;
        }
        if( bInheritHandle ) {
            startup.dwFlags |= STARTF_USESTDHANDLES;
        }
    }

    DWORD flags = 0;

    usize cmd_line_utf8_len = 0;
    for( usize i = 0; i < cmd.count; ++i ) {
        cmd_line_utf8_len += strlen( cmd.args[i] );
    }

    usize wide_cmd_line_cap  = cmd_line_utf8_len + 8 + cmd.count;
    usize wide_cmd_line_size = sizeof(wchar_t) * wide_cmd_line_cap;
    usize wide_cmd_line_len  = 0;
    wchar_t* cmd_line = HeapAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, wide_cmd_line_size );

    expect( cmd_line, "failed to allocate command line wide buffer!" );

    for( usize i = 0; i < cmd.count; ++i ) {
        String current = string_from_cstr( cmd.args[i] );
        if( !current.cc || !current.len ) {
            continue;
        }

        b32 contains_space = false;
        if( string_find( current, ' ', 0 ) ) {
            cmd_line[wide_cmd_line_len++] = L'"';
            contains_space = true;
        }

        int len = MultiByteToWideChar(
            CP_UTF8, 0, current.cc, current.len, 
            cmd_line + wide_cmd_line_len, wide_cmd_line_cap - wide_cmd_line_len );
        wide_cmd_line_len += len;

        if( contains_space ) {
            cmd_line[wide_cmd_line_len++] = L'"';
        }

        if( i + 1 != cmd.count ) {
            cmd_line[wide_cmd_line_len++] = L' ';
        }
    }

    wchar_t* wide_cwd = NULL;
    if( opt_cwd ) {
        wide_cwd = win32_local_path_canon( string_from_cstr( opt_cwd ) );
        cb_info( "cd '%S'", wide_cwd );
    }

    cb_info( "%S", cmd_line );
    BOOL res = CreateProcessW(
        NULL, cmd_line, NULL, NULL, bInheritHandle,
        flags, NULL, wide_cwd, &startup, &info );
    HeapFree( GetProcessHeap(), 0, cmd_line );

    expect( res, "failed to launch process '%s'!", cmd.args[0] );

    CloseHandle( info.hThread );
    return (isize)info.hProcess;
}
int process_wait( PID pid ) {
    DWORD res = WaitForSingleObject( (HANDLE)pid, INFINITE );
    switch( res ) {
        case WAIT_OBJECT_0: break;
        default: {
            String reason = win32_local_error_message( GetLastError() );
            panic( "failed to wait for pid! reason: %s", reason.cc );
            return -1;
        } break;
    }

    DWORD exit_code = 0;
    expect( GetExitCodeProcess(
        (HANDLE)pid, &exit_code ), "failed to get exit code!" );

    CloseHandle( (HANDLE)pid );
    return exit_code;
}
b32 process_wait_timed( PID pid, int* opt_out_res, u32 ms ) {
    b32 success = true;
    DWORD res   = WaitForSingleObject( (HANDLE)pid, ms );
    switch( res ) {
        case WAIT_OBJECT_0: break;
        case WAIT_TIMEOUT: {
            success = false;
        } break;
        default: {
            String reason = win32_local_error_message( GetLastError() );
            panic( "failed to wait for pid! reason: %s", reason.cc );
            return -1;
        } break;
    }

    DWORD exit_code = 0;
    expect( GetExitCodeProcess(
        (HANDLE)pid, &exit_code ), "failed to get exit code!" );

    if( success ) {
        CloseHandle( (HANDLE)pid );
    }

    *opt_out_res = exit_code;
    return success;
}
void process_discard( PID pid ) {
    CloseHandle( (HANDLE)pid );
}

unsigned int win32_thread_proc( void* params ) {
    struct Win32ThreadParams* p = params;
    global_thread_id = p->id;
    
    fence();
    p->proc( p->params );
    fence();

    return 0;
}

void thread_create( JobFN* func, void* params ) {
    expect(
        global_thread_id_source < (CBUILD_THREAD_COUNT + 1),
        "exceeded maximum number of threads!" );

    u32 id = atomic_add( &global_thread_id_source, 1 );

    struct Win32ThreadParams* p = global_win32_thread_params + (id - 1);
    p->id     = id;
    p->params = params;
    p->proc   = func;

    fence();

    HANDLE h = (HANDLE)_beginthreadex( 0, 0, win32_thread_proc, p, 0, 0 );
    expect( h != NULL, "failed to create thread!" );
}

char* internal_cwd(void) {
    DWORD len = GetCurrentDirectoryA( 0, 0 );
    char* buf = memory_alloc( len );

    expect( buf, "failed to allocate working directory buffer!" );

    GetCurrentDirectoryA( len, buf );

    for( usize i = 0; i < len; ++i ) {
        if( buf[i] == '\\' ) {
            buf[i] = '/';
        }
    }

    return buf;
}
char* internal_home(void) {
    const char* drive = getenv( "HOMEDRIVE" );
    expect( drive, "failed to get home directory drive!" );
    const char* home  = getenv( "HOMEPATH" );
    expect( home, "failed to get home path!" );

    DString* buf = dstring_concat_cstr( drive, home );
    expect( buf, "failed to allocate home directory buffer!" );

    usize len = dstring_len( buf );
    for( usize i = 0; i < len; ++i ) {
        if( buf[i] == '\\' ) {
            buf[i] = '/';
        }
    }

    return buf;
}

#else /* WINDOWS end */
#include <unistd.h>
#include <limits.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>

volatile atom global_semaphore_val = 0;

struct PosixMutex {
    atom value;
#if defined(ARCH_64BIT)
    u32  __padding;
#endif
};

struct PosixThreadParams {
    JobFN* proc;
    void*  params;
    u32    id;
};
static struct PosixThreadParams global_posix_thread_params[CBUILD_THREAD_COUNT];

void _platform_init_(void) {
    return;
}

static struct timespec ms_to_timespec( u32 ms ) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    return ts;
}
static f64 timespec_to_ms( struct timespec* ts ) {
    return ((f64)ts->tv_sec * 1000.0) + ((f64)ts->tv_nsec / 1000000.0);
}
static const char* generate_semaphore_name(void) {
    atom val = atomic_add( &global_semaphore_val, 1 );
    return (const char*)local_fmt( "cbuild_sem%u", val );
}

void* memory_alloc( usize size ) {
    void* res = malloc( size );
    if( !res ) {
        return NULL;
    }
    memset( res, 0, size );
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, size );
        atomic_add64( &global_total_memory_usage, size );
    } else {
        global_memory_usage       += size;
        global_total_memory_usage += size;
    }
    return res;
}
void* memory_realloc( void* memory, usize old_size, usize new_size ) {
    assertion( new_size >= old_size, "attempted to reallocate to smaller buffer!" );
    void* res = realloc( memory, new_size );
    if( !res ) {
        return NULL;
    }
    usize diff = new_size - old_size;
    memset( res + old_size, 0, diff );
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, diff );
        atomic_add64( &global_total_memory_usage, diff );
    } else {
        global_memory_usage       += diff;
        global_total_memory_usage += diff;
    }
    return res;
}
void memory_free( void* memory, usize size ) {
    if( !memory ) {
        cb_warn( "attempted to free null pointer!" );
        return;
    }
    free( memory );
    atom64 neg = size;
    neg = -neg;
    if( global_is_mt ) {
        atomic_add64( &global_memory_usage, neg );
    } else {
        global_memory_usage += neg;
    }
}
b32 path_is_absolute( const char* path ) {
    return *path == '/';
}
b32 path_exists( const char* path ) {
    return access( path, F_OK ) == 0;
}
b32 path_is_directory( const char* path ) {
    struct stat s;
    int res = stat( path, &s );
    if( res == -1 ) {
        return false;
    }

    return S_ISDIR( s.st_mode );
}
static b32 path_walk_dir_internal(
    DString** path, b32 recursive, b32 include_dirs,
    usize* out_count, DString** out_buffer
) {
    struct dirent* entry;
    DIR* dir = opendir( *path );
    if( !dir ) {
        return false;
    }

    usize original_len = dstring_len( *path );
    long  pos = 0;

    while( (entry = readdir( dir )) ) {
        if(
            strcmp( entry->d_name, "." )    == 0 ||
            strcmp( entry->d_name, ".." )   == 0 ||
            strcmp( entry->d_name, ".git" ) == 0
        ) {
            continue;
        }
        DString* _new = dstring_push( *path, '/' );
        if( !_new ) {
            closedir( dir );
            return false;
        }
        *path = _new;

        usize name_len = strlen( entry->d_name );
        _new = dstring_append( *path, string_new( name_len, entry->d_name ) );
        if( !_new ) {
            closedir( dir );
            return false;
        }
        *path = _new;

        struct stat st;
        stat( *path, &st );

        if( S_ISDIR( st.st_mode ) ) {
            if( include_dirs ) {
                _new = dstring_append(
                    *out_buffer, string_new( dstring_len( *path ) + 1, *path ) );
                if( !_new ) {
                    closedir( dir );
                    return false;
                }
                *out_buffer = _new;

                *out_count += 1;
            }

            if( recursive ) {
                pos = telldir( dir );
                closedir( dir );

                if( !path_walk_dir_internal(
                    path, recursive, include_dirs,
                    out_count, out_buffer
                ) ) {
                    return false;
                }

                dstring_truncate( *path, original_len );
                dir = opendir( *path );

                seekdir( dir, pos );
            }

            dstring_truncate( *path, original_len );
            continue;
        }

        _new = dstring_append(
            *out_buffer, string_new( dstring_len( *path ) + 1, *path ) );
        if( !_new ) {
            closedir( dir );
            return false;
        }
        *out_buffer = _new;

        *out_count += 1;

        dstring_truncate( *path, original_len );
    }

    closedir( dir );
    return true;
}

b32 fd_open( const char* path, FileOpenFlags flags, FD* out_file ) {
    if( !validate_file_flags( flags ) ) {
        return false;
    }
    int oflag = 0;
    if( (flags & (FOPEN_READ | FOPEN_WRITE)) == FOPEN_READ ) {
        oflag |= O_RDONLY;
    } else if( (flags & (FOPEN_READ | FOPEN_WRITE)) == FOPEN_WRITE ) {
        oflag |= O_WRONLY;
    } else { // read + write
        oflag |= O_RDWR;
    }
    if( flags & FOPEN_TRUNCATE ) {
        oflag |= O_TRUNC;
    }
    if( flags & FOPEN_CREATE ) {
        oflag |= O_CREAT;
    }
    if( flags & FOPEN_APPEND ) {
        oflag |= O_APPEND;
    }

    mode_t mode = S_IRUSR | S_IWUSR;

    int fd = open( path, oflag, mode );
    if( fd < 0 ) {
        cb_error( "fd_open: failed to open '%s'!", path );
        return false;
    }

    *out_file = fd;
    return true;
}
void fd_close( FD* file ) {
    close( *file );
    *file = 0;
}
b32 fd_write( FD* file, usize size, const void* buf, usize* opt_out_write_size ) {
    isize write_size = write( *file, buf, size );
    if( write_size < 0 ) {
        return false;
    }

    if( opt_out_write_size ) {
        *opt_out_write_size = write_size;
    }
    return true;
}
b32 fd_read( FD* file, usize size, void* buf, usize* opt_out_read_size ) {
    isize read_size = read( *file, buf, size );
    if( read_size < 0 ) {
        return false;
    }

    if( opt_out_read_size ) {
        *opt_out_read_size = read_size;
    }

    return true;
}
b32 fd_truncate( FD* file ) {
    usize pos = fd_query_position( file ); // fd_query_position handles lseek fail
    return ftruncate( *file, pos ) == 0;
}
usize fd_query_size( FD* file ) {
    off_t pos = lseek( *file, 0, SEEK_CUR );
    expect( pos >= 0, "failed to query file descriptor size!" );
    off_t res = lseek( *file, 0, SEEK_END );
    expect( res >= 0, "failed to query file descriptor size!" );
    expect( lseek( *file, pos, SEEK_SET ) >= 0,
        "failed to query file descriptor size!" );

    return res;
}
void fd_seek( FD* file, FileSeek type, isize seek ) {
    static const int local_whence[] = {
        SEEK_CUR, // FSEEK_CURRENT
        SEEK_SET, // FSEEK_BEGIN
        SEEK_END, // FSEEK_END
    };
    expect( lseek( *file, seek, local_whence[type] ) >= 0, "failed to seek!" );
}
usize fd_query_position( FD* file ) {
    off_t pos = lseek( *file, 0, SEEK_CUR );
    expect( pos >= 0, "failed to get current file position!" );
    return pos;
}
time_t file_query_time_create( const char* path ) {
    struct stat st;
    expect( stat( path, &st ) == 0,
        "failed to query create time for '%s'!", path );
    return st.st_ctime;
}
time_t file_query_time_modify( const char* path ) {
    struct stat st;
    expect( stat( path, &st ) == 0,
        "failed to query modify time for '%s'!", path );
    return st.st_mtime;
}
b32 file_move( const char* dst, const char* src ) {
    return rename( src, dst ) == 0;
}
b32 file_copy( const char* dst, const char* src ) {
    FD src_file, dst_file;
    if( !fd_open( src, FOPEN_READ, &src_file ) ) {
        return false;
    }
    if( path_exists( dst ) ) {
        if( !fd_open( dst, FOPEN_WRITE | FOPEN_TRUNCATE, &dst_file ) ) {
            fd_close( &src_file );
            return false;
        }
    } else {
        if( !fd_open( dst, FOPEN_WRITE | FOPEN_CREATE, &dst_file ) ) {
            fd_close( &src_file );
            return false;
        }
    }

    char* buf = (char*)local_byte_buffer();
    usize rem = fd_query_size( &src_file );

    while( rem ) {
        usize max_write = rem;
        if( rem > CBUILD_LOCAL_BUFFER_CAPACITY ) {
            max_write = CBUILD_LOCAL_BUFFER_CAPACITY;
        }

        if( !fd_read( &src_file, max_write, buf, 0 ) ) {
            fd_close( &src_file );
            fd_close( &dst_file );
            return false;
        }
        if( !fd_write( &dst_file, max_write, buf, 0 ) ) {
            fd_close( &src_file );
            fd_close( &dst_file );
            return false;
        }

        rem -= max_write;
    }

    fd_close( &src_file );
    fd_close( &dst_file );
    return true;
}
b32 file_remove( const char* path ) {
    int res = remove( path );
    return res == 0;
}
b32 dir_create( const char* path ) {
    int res = mkdir( path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    if( res == 0 ) {
        return true;
    }
    switch( errno ) {
        case EEXIST: return true;
        default:     return false;
    }
}
static b32 dir_remove_internal( const char* path ) {
    int res = rmdir( path );
    return res == 0;
}

atom atomic_add( atom* atomic, atom val ) {
    return __sync_fetch_and_add( atomic, val );
}
atom64 atomic_add64( atom64* atom, atom64 val ) {
    return __sync_fetch_and_add( atom, val );
}
atom atomic_compare_swap( atom* atomic, atom comp, atom exch ) {
    return __sync_val_compare_and_swap( atomic, comp, exch );
}
atom64 atomic_compare_swap64( atom64* atom, atom64 comp, atom64 exch ) {
    return __sync_val_compare_and_swap( atom, comp, exch );
}
void fence(void) {
#if defined(ARCH_X86)
    __asm__ volatile ("mfence":::"memory");
#elif defined(ARCH_ARM)
    __asm__ volatile ("dmb":::"memory");
#else
    __asm__ volatile ("":::"memory");
#endif
}

b32 mutex_create( Mutex* out_mutex ) {
    struct PosixMutex* mtx = (struct PosixMutex*)out_mutex;
    mtx->value = 0;
    return true;
}
b32 mutex_is_valid( const Mutex* mutex ) {
    unused( mutex );
    return true;
}
void mutex_lock( Mutex* mutex ) {
    struct PosixMutex* mtx = (struct PosixMutex*)mutex;
    while( atomic_compare_swap( &mtx->value, 0, mtx->value + 1 ) != 0 ) {
        thread_sleep(1);
    }
}
b32 mutex_lock_timed( Mutex* mutex, u32 ms ) {
    if( ms == MT_WAIT_INFINITE ) {
        mutex_lock( mutex );
        return true;
    }

    struct PosixMutex* mtx = (struct PosixMutex*)mutex;

    for( u32 i = 0; i < ms; ++i ) {
        if( atomic_compare_swap( &mtx->value, 0, mtx->value + 1 ) != 0 ) {
            thread_sleep(1);
            continue;
        } else {
            return true;
        }
    }
    return false;
}
void mutex_unlock( Mutex* mutex ) {
    struct PosixMutex* mtx = (struct PosixMutex*)mutex;
    atomic_compare_swap( &mtx->value, mtx->value, mtx->value - 1 );
}
void mutex_destroy( Mutex* mutex ) {
    mutex->handle = 0;
}

b32 semaphore_create( Semaphore* out_semaphore ) {
    sem_t* s = sem_open( generate_semaphore_name(), O_CREAT, 0644, 0 );
    if( !s ) {
        const char* reason = "unknown";

        // reason for not handling case is commented below.
        // technically all of these errors should lead
        // to program being aborted but that's left up
        // to the programmer to decide in this case.
        switch( errno ) {
            case ENFILE:
            case EMFILE: {
                reason = "too many open file descriptors.";
            } break;
            case ENOMEM: {
                reason = "out of memory.";
            } break;
            case EACCES: // this function does not open existing semaphores.
            case EEXIST: // this function should not be capable of creating
                         // semaphore with existing name.
            case EINVAL: // value is never greater than SEM_VALUE_MAX
            case ENAMETOOLONG: // name can never be too long
            case ENOENT: // same as EACCES
            default: break;
        }

        cb_error( "failed to create semaphore! reason: %s", reason );
        return false;
    }
    out_semaphore->handle = s;
    return true;
}
b32 semaphore_is_valid( const Semaphore* semaphore ) {
    return semaphore->handle != NULL;
}
void semaphore_wait( Semaphore* semaphore ) {
    expect( sem_wait( semaphore->handle ) == 0, "failed to wait on semaphore!" );
}
b32 semaphore_wait_timed( Semaphore* semaphore, u32 ms ) {
    if( ms == MT_WAIT_INFINITE ) {
        semaphore_wait( semaphore );
        return true;
    }

    struct timespec ts = ms_to_timespec( ms );
    int res = sem_timedwait( semaphore->handle, &ts );

    if( res == ETIMEDOUT ) {
        // only error that is actually expected,
        // anything else should abort the program
        return false;
    } else {
        expect( res == 0, "failed to wait on semaphore!" );
        return true;
    }
}
void semaphore_signal( Semaphore* semaphore ) {
    expect( sem_post( semaphore->handle ) == 0, "failed to post semaphore!" );
}
void semaphore_destroy( Semaphore* semaphore ) {
    sem_close( semaphore->handle );
    *semaphore = semaphore_null();
}

void pipe_open( ReadPipe* out_read, WritePipe* out_write ) {
    int fd[2];
    expect( pipe( fd ) != -1, "failed to create pipes!" );
    *out_read  = fd[0];
    *out_write = fd[1];
}
void pipe_close( Pipe pipe ) {
    close( pipe );
}

b32 process_in_path( const char* process_name ) {
    Command cmd = command_new( "which", process_name );

    PID pid = process_exec( cmd, true, 0, 0, 0, 0 );
    int res = process_wait( pid );

    return res == 0;
}
void posix_process_replace( Command cmd ) {
    execvp( cmd.args[0], (char* const*)cmd.args );
}
PID process_exec(
    Command cmd, b32 redirect_void, ReadPipe* opt_stdin,
    WritePipe* opt_stdout, WritePipe* opt_stderr, const char* opt_cwd
) {
    ReadPipe   stdin_;
    WritePipe stdout_;
    WritePipe stderr_;

    if( redirect_void ) {
        volatile struct GlobalBuffers* gb = get_global_buffers();
        stdin_  = gb->void_read;
        stdout_ = gb->void_write;
        stderr_ = gb->void_write;
    } else {
        stdin_  = opt_stdin  ? *opt_stdin  : STDIN_FILENO;
        stdout_ = opt_stdout ? *opt_stdout : STDOUT_FILENO;
        stderr_ = opt_stderr ? *opt_stderr : STDERR_FILENO;
    }

    pid_t pid = fork();
    expect( pid >= 0, "failed to fork!" );

    if( pid ) { // thread that ran process_exec
        if( opt_cwd ) {
            cb_info( "cd '%s'", opt_cwd );
        }
        DString* flat = command_flatten_dstring( &cmd );
        if( flat ) {
            cb_info( "%s", flat );
            dstring_free( flat );
        }
        return pid;
    }

    // thread where process will run

    expect_crash( dup2( stdin_ , STDIN_FILENO  ) >= 0, "failed to setup stdin!" );
    expect_crash( dup2( stdout_, STDOUT_FILENO ) >= 0, "failed to setup stdout!" );
    expect_crash( dup2( stderr_, STDERR_FILENO ) >= 0, "failed to setup stderr!" );

    if( opt_cwd ) {
        chdir( opt_cwd );
    }

    expect_crash( execvp(
        cmd.args[0], (char* const*)(cmd.args) // cmd.args always has a null string at the end
    ) >= 0, "failed to execute command!" );
    exit(0);
}
int process_wait( PID pid ) {
    int wstatus = 0;
    expect( waitpid( pid, &wstatus, 0 ) == pid, "failed to wait for process!" );

    if( WIFEXITED( wstatus ) ) {
        int status = WEXITSTATUS( wstatus );
        return status;
    }

    return -1;
}
b32 process_wait_timed( PID pid, int* opt_out_res, u32 ms ) {
    if( ms == MT_WAIT_INFINITE ) {
        int res = process_wait( pid );
        if( opt_out_res ) {
            *opt_out_res = res;
        }
        return true;
    }

    for( u32 i = 0; i < ms; ++i ) {
        int wstatus = 0;
        pid_t v = waitpid( pid, &wstatus, WNOHANG );
        expect( v != -1, "failed to wait for process!" );

        if( v == 0 ) {
            thread_sleep(1);
            continue;
        }

        if( opt_out_res ) {
            if( WIFEXITED( wstatus ) ) {
                *opt_out_res = WEXITSTATUS( wstatus );
            } else {
                *opt_out_res = -1;
            }
        }

        return true;
    }

    return false;
}
void process_discard( PID pid ) {
    unused(pid); // this is not necessary on POSIX
}

f64 timer_milliseconds(void) {
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    fence();
    return timespec_to_ms( &ts );
}
f64 timer_seconds(void) {
    struct timespec ts;
    clock_gettime( CLOCK_MONOTONIC, &ts );
    fence();
    return timespec_to_ms( &ts ) / 1000.0;
}

void thread_sleep( u32 ms ) {
    struct timespec ts = ms_to_timespec( ms );
    struct timespec rem;
    memory_zero( &rem, sizeof(rem) );

    while( clock_nanosleep( CLOCK_REALTIME, 0, &ts, &rem ) ) {
        ts = rem;
    }
}

void* posix_thread_proc( void* params ) {
    struct PosixThreadParams* p = params;
    global_thread_id = p->id;

    fence();
    p->proc( p->params );
    fence();

    return 0;
}

void thread_create( JobFN* func, void* params ) {
    expect(
        global_thread_id_source < (CBUILD_THREAD_COUNT + 1),
        "exceeded maximum number of threads! max: %u", CBUILD_THREAD_COUNT );

    u32 id = atomic_add( &global_thread_id_source, 1 );

    struct PosixThreadParams* p = global_posix_thread_params + (id - 1);
    p->id     = id;
    p->params = params;
    p->proc   = func;

    fence();

    pthread_t thread;
    int res = pthread_create( &thread, NULL, posix_thread_proc, p );
    expect( res == 0, "failed to create thread!" );
}

char* internal_cwd(void) {
    char* buf = memory_alloc( PATH_MAX );
    expect( buf, "failed to allocate working directory buffer!" );

    char* res = getcwd( buf, PATH_MAX );
    expect( res, "failed to get working directory!" );

    return res;
}
char* internal_home(void) {
    char* home = getenv( "HOME" );
    expect( home, "failed to get home directory!" );

    return home;
}

#endif /* POSIX end */

#endif /* CBUILD_IMPLEMENTATION end */
