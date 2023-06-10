#ifndef OFFSETS_H
#define OFFSETS_H

#include "pch.h"

extern struct DynamicOffsets {
    // NOLINTBEGIN(readability-identifier-naming)

#if DEBUG_VARIABLES
    asr_utils::HexVariable<ptrdiff_t> PlayerController{"PlayerController"};
    asr_utils::HexVariable<ptrdiff_t> TimePlayedSeconds{"TimePlayedSeconds"};
    asr_utils::HexVariable<ptrdiff_t> TimePlayedSecondsLoadedFromSaveGame{
        "TimePlayedSecondsLoadedFromSaveGame"};
    asr_utils::HexVariable<ptrdiff_t> PlayerMissionComponent{"PlayerMissionComponent"};
    asr_utils::HexVariable<ptrdiff_t> CachedPlaythroughIndex{"CachedPlaythroughIndex"};
    asr_utils::HexVariable<ptrdiff_t> MissionPlaythroughs{"MissionPlaythroughs"};
    asr_utils::HexVariable<size_t> MissionPlaythroughs_ElementSize{
        "MissionPlaythroughs_ElementSize"};
    asr_utils::HexVariable<ptrdiff_t> MissionList{"MissionList"};
    asr_utils::HexVariable<size_t> MissionList_ElementSize{"MissionList_ElementSize"};
    asr_utils::HexVariable<ptrdiff_t> MissionClass{"MissionClass"};
    asr_utils::HexVariable<ptrdiff_t> ActiveObjectiveSet{"ActiveObjectiveSet"};
    asr_utils::HexVariable<ptrdiff_t> ObjectivesProgress{"ObjectivesProgress"};
    asr_utils::HexVariable<size_t> ObjectivesProgress_ElementSize{"ObjectivesProgress_ElementSize"};
#else
    ptrdiff_t PlayerController;
    ptrdiff_t TimePlayedSeconds;
    ptrdiff_t TimePlayedSecondsLoadedFromSaveGame;
    ptrdiff_t PlayerMissionComponent;
    ptrdiff_t CachedPlaythroughIndex;
    ptrdiff_t MissionPlaythroughs;
    size_t MissionPlaythroughs_ElementSize;
    ptrdiff_t MissionList;
    size_t MissionList_ElementSize;
    ptrdiff_t MissionClass;
    ptrdiff_t ActiveObjectiveSet;
    ptrdiff_t ObjectivesProgress;
    size_t ObjectivesProgress_ElementSize;
#endif

    // NOLINTEND(readability-identifier-naming)
} offsets;

/**
 * @brief Resets all found offsets after attaching to a new game.
 */
void reset_offsets(void);

/**
 * @brief Tries to find the dynamic offsets we use.
 *
 * @return True if all offsets have been found, false if still in progress.
 */
bool find_offsets(void);

/**
 * @brief Checks if we've found all required dynamic offsets.
 *
 * @return True if all offsets have been found, false if still in progress.
 */
bool found_all_offsets(void);

#endif /* OFFSETS_H */
