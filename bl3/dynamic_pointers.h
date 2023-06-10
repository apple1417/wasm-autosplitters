#ifndef DYNAMIC_POINTERS_H
#define DYNAMIC_POINTERS_H

#include "pch.h"
#include "unreal_types.h"

/**
 * @brief Current playthrough index watcher.
 */
extern asr_utils::MemWatcher<int32_t> playthrough;

/**
 * @brief Play time watchers - should be summed.
 */
extern asr_utils::MemWatcher<int32_t> time_played;
extern asr_utils::MemWatcher<int32_t> time_played_save_game;

/**
 * @brief Current playthrough's mission count watcher.
 */
extern asr_utils::MemWatcher<decltype(TArray<void>::count)> mission_count;

/**
 * @brief Watcher for the "picked up echo" objective to start the run.
 */
extern asr_utils::MemWatcher<int32_t> starting_echo;

/**
 * @brief Performs inital initalization of the dynamic pointers on first attaching to a new game.
 */
void init_dynamic_pointers(void);

/**
 * @brief Updates the relevant pointers after a playthrough change
 */
void on_playthrough_changed(void);

/**
 * @brief Updates the relevant pointers after a mission count change
 */
void on_missions_changed(void);

#endif /* DYNAMIC_POINTERS_H */
