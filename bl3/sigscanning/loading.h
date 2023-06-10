#ifndef LOADING_H
#define LOADING_H

#include "pch.h"

/**
 * @brief Loading status watcher.
 */
extern asr_utils::MemWatcher<bool> loading;

/**
 * @brief Tries to find the loading pointer on attaching to a game.
 *
 * @return True if successful.
 */
bool find_loading(void);

#endif /* LOADING_H */
