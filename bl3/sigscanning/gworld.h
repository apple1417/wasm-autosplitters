#ifndef GWORLD_H
#define GWORLD_H

#include "pch.h"
#include "unreal_types.h"

/**
 * @brief GWorld name index watcher.
 */
extern asr_utils::MemWatcher<decltype(FName::index)> gworld_name;

/**
 * @brief GWorld name as a string.
 */
extern asr_utils::Variable<std::string> current_world;

/**
 * @brief Tries to find gworld on attaching to a game.
 *
 * @return True if successful.
 */
bool find_gworld(void);

#endif /* GWORLD_H */
