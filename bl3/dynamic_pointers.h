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
 * @brief Watcher for the objective set name for each of the relevant missions.
 */
extern asr_utils::MemWatcher<decltype(FName::index)> tyreen_cutscene_objective_set;
extern asr_utils::MemWatcher<decltype(FName::index)> jackpot_cutscene_objective_set;
extern asr_utils::MemWatcher<decltype(FName::index)> wedding_cutscene_objective_set;
extern asr_utils::MemWatcher<decltype(FName::index)> bounty_cutscene_objective_set;
extern asr_utils::MemWatcher<decltype(FName::index)> krieg_cutscene_objective_set;

/**
 * @brief A list of watchers for missions which are currently incomplete.
 */
extern std::vector<asr_utils::MemWatcher<uint8_t>> incomplete_missions;

/**
 * @brief A variable holding the amount size of the incomplete mission list.
 * @note Should be updated when removing entries.
 */
extern asr_utils::Variable<size_t> incomplete_mission_count;

/**
 * @brief Performs initial initialization of the dynamic pointers on first attaching to a new game.
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
