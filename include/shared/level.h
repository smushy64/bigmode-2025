#if !defined(SHARED_LEVEL_H)
#define SHARED_LEVEL_H
/**
 * @file   level.h
 * @brief  Level conditions.
 * @author Alicia Amarilla (smushyaa@gmail.com)
 * @date   February 02, 2025
*/
#include <stdint.h>

enum class LevelCondition : uint16_t {
    NONE,
    DEFEAT_ENEMIES,
    COLLECT_BATTERIES,
    DEFEAT_ENEMIES_AND_COLLECT_BATTERIES,

    COUNT
};
inline
const char* to_string( LevelCondition condition ) {
    switch( condition ) {
        case LevelCondition::NONE:
            return "None.";
        case LevelCondition::DEFEAT_ENEMIES:
            return "Defeat Enemies.";
        case LevelCondition::COLLECT_BATTERIES:
            return "Collect Batteries.";
        case LevelCondition::DEFEAT_ENEMIES_AND_COLLECT_BATTERIES:
            return "Enemies + Batteries.";
        case LevelCondition::COUNT: break;
    }
    return "";
}
inline
const char* get_description( LevelCondition condition ) {
    switch( condition ) {
        case LevelCondition::NONE:              return "Reach the Exit.";
        case LevelCondition::DEFEAT_ENEMIES:    return "Defeat all enemies.";
        case LevelCondition::COLLECT_BATTERIES: return "Collect all batteries.";
        case LevelCondition::DEFEAT_ENEMIES_AND_COLLECT_BATTERIES:
            return "Defeat all enemies and collect all batteries.";
        case LevelCondition::COUNT: return "";
    }
    return "";
}

#endif /* header guard */
