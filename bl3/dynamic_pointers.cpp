#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "settings.h"
#include "sigscanning/gnames.h"
#include "sigscanning/gworld.h"
#include "sigscanning/localplayer.h"
#include "unreal_types.h"

asr_utils::MemWatcher<int32_t> playthrough{
    {},
    asr_utils::make_variable<int>(DEBUG_VARIABLES ? "Playthrough" : "")};
asr_utils::MemWatcher<int32_t> time_played{};
asr_utils::MemWatcher<int32_t> time_played_save_game{};
asr_utils::MemWatcher<decltype(TArray<void>::count)> mission_count{
    {},
    asr_utils::make_variable<decltype(TArray<void>::count)>("Mission Count")};

asr_utils::MemWatcher<int32_t> starting_echo{
    {},
    asr_utils::make_variable<int>(DEBUG_VARIABLES ? "Starting Echo" : "")};

asr_utils::MemWatcher<decltype(FName::index)> tyreen_cutscene_objective_set{};
asr_utils::MemWatcher<decltype(FName::index)> jackpot_cutscene_objective_set{};
asr_utils::MemWatcher<decltype(FName::index)> wedding_cutscene_objective_set{};
asr_utils::MemWatcher<decltype(FName::index)> bounty_cutscene_objective_set{};
asr_utils::MemWatcher<decltype(FName::index)> krieg_cutscene_objective_set{};

namespace {

// Name, Objective Index, Suppress Changed, WATCHER
const constinit std::array<std::tuple<const char*, size_t, bool, asr_utils::MemWatcher<int32_t>&>, 1>
    OBJECTIVE_WATCHERS{{
        {"Mission_Ep01_ChildrenOfTheVault_C", 4, true, starting_echo},
    }};

// Name, Watcher
const constinit std::array<std::pair<const char*, asr_utils::MemWatcher<int32_t>&>, 5>
    OBJECTIVE_SET_WATCHERS{{
        {"Mission_Ep23_TyreenFinalBoss_C", tyreen_cutscene_objective_set},
        {"Mission_DLC1_Ep07_TheHeist_C", jackpot_cutscene_objective_set},
        {"EP06_DLC2_C", wedding_cutscene_objective_set},
        {"Mission_Ep05_Crater_C", bounty_cutscene_objective_set},
        {"ALI_EP05_C", krieg_cutscene_objective_set},
    }};

// Name, Setting
const constinit std::array<std::pair<const char*, const bool Settings::*>, 5> INSTANT_STARTING_MISSIONS{{
    {"Mission_DLC1_Ep01_MeetTimothy_C", &Settings::start_jackpot},
    {"EP01_DLC2_C", &Settings::start_wedding},
    {"Mission_Ep01_WestlandWelcome_C", &Settings::start_bounty},
    {"ALI_EP01_C", &Settings::start_krieg},
    {"Mission_GearUp_Intro_C", &Settings::start_arms_race},
}};

/**
 * @brief Gets the offset into the mission playthroughs array to use for the current playthrough.
 * @note Handles out of range values, such as the -1 when there's no cached playthrough.
 *
 * @return The current playthrough offset.
 */
ptrdiff_t current_playthrough_offset(void) {
    auto playthrough_idx = playthrough.current() == 1 ? 1 : 0;
    return static_cast<ptrdiff_t>(offsets.MissionPlaythroughs_ElementSize * playthrough_idx);
}

/**
 * @brief Checks if the given mission is one we track objectives on, and sets up its watchers.
 *
 * @param mission_name The mission's name.
 * @param playthrough_mission_list_offset The current mission list's offset in the playthrough data.
 * @param this_mission_objectives_data_offset The mission's objective data pointer's offset in the
 *                                            current mission list.
 * @return True if the mission matches one we track.
 */
bool match_objective_missions(const std::string_view& mission_name,
                              ptrdiff_t playthrough_mission_list_offset,
                              ptrdiff_t this_mission_objectives_data_offset) {
    for (auto& [name, idx, suppress, watcher] : OBJECTIVE_WATCHERS) {
        if (mission_name == name) {
            watcher.pointer() = {
                base_localplayer,
                {
                    offsets.PlayerController,
                    offsets.PlayerMissionComponent,
                    offsets.MissionPlaythroughs,
                    playthrough_mission_list_offset,
                    this_mission_objectives_data_offset,
                    static_cast<ptrdiff_t>(offsets.ObjectivesProgress_ElementSize * idx),
                }};

            if (suppress) {
                watcher.update(game_info);
                watcher.suppress_changed();
            }

            return true;
        }
    }

    return false;
}

/**
 * @brief Checks if the given mission is one we track objective sets on, and sets up its watchers.
 *
 * @param mission_name The mission's name.
 * @param playthrough_mission_list_offset The current mission list's offset in the playthrough data.
 * @param this_mission_objective_set_offset The mission's objective set pointer's offset in the
 *                                          current mission list.
 * @return True if the mission matches one we track.
 */
bool match_objective_set_missions(const std::string_view& mission_name,
                                  ptrdiff_t playthrough_mission_list_offset,
                                  ptrdiff_t this_mission_objective_set_offset) {
    for (auto& [name, watcher] : OBJECTIVE_SET_WATCHERS) {
        if (mission_name == name) {
            watcher.pointer() = {base_localplayer,
                                 {
                                     offsets.PlayerController,
                                     offsets.PlayerMissionComponent,
                                     offsets.MissionPlaythroughs,
                                     playthrough_mission_list_offset,
                                     this_mission_objective_set_offset,
                                     offsetof(UObject, name.index),
                                 }};
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks if the given mission is one we start the timer on, and does so if needed.
 *
 * @param mission_name The mission's name.
 * @param this_mission_objectives_data The address of the current mission's objectives data pointer.
 * @return True if the mission matches one we track.
 */
bool match_instant_start_missions(const std::string_view& mission_name,
                                  Address this_mission_objectives_data) {
    bool found_starting_mission = false;
    for (auto& [name, setting] : INSTANT_STARTING_MISSIONS) {
        if (mission_name == name && settings.*setting) {
            found_starting_mission = true;
            break;
        }
    }
    if (!found_starting_mission) {
        return false;
    }

    /*
    Detect picking up the missions we start on by looking for an incomplete first objective.

    This isn't perfect, but it's relatively simple and works for what we need it to. If we tried
    tracking what missions we had last update, we'd also need to track what character you've got
    selected, which also has side cases like deleting your char and making a new one with the same
    save game id, it just gets messy.

    Picking up a mission or loading into an ungeared save will have the first objective incomplete,
    so will get picked up by this, while it won't pick up loading into a save where you've already
    finished the dlc for the first time. The only side case is loading a save where you picked up
    one of the missions but didn't complete anything - i.e. bounty. There's no real way to tell the
    difference between this and loading an ungeared save though, it only happens in sancturary, and
    you can always just switch it off if it's really a problem.
    */

    auto first_objective = asr_utils::read_mem<int32_t>(
        game_info, asr_utils::read_address(game_info, this_mission_objectives_data)
                       + (0 * offsets.ObjectivesProgress_ElementSize));

    if (first_objective == 0) {
        runtime_print_message("Starting run due to picking up mission {}", mission_name);
        timer_start();
    }

    return true;
}

}  // namespace

void init_dynamic_pointers(void) {
    playthrough.pointer() = {
        base_localplayer,
        {offsets.PlayerController, offsets.PlayerMissionComponent, offsets.CachedPlaythroughIndex}};
    playthrough.update(game_info);

    time_played.pointer() = {base_localplayer,
                             {offsets.PlayerController, offsets.TimePlayedSeconds}};
    time_played.update(game_info);

    time_played_save_game.pointer() = {
        base_localplayer, {offsets.PlayerController, offsets.TimePlayedSecondsLoadedFromSaveGame}};
    time_played_save_game.update(game_info);

    playthrough.suppress_changed();
    on_playthrough_changed();
}

void on_playthrough_changed(void) {
    runtime_print_message("Playthrough changed {} -> {}", playthrough.old(), playthrough.current());

    mission_count.pointer() = {
        base_localplayer,
        {offsets.PlayerController, offsets.PlayerMissionComponent, offsets.MissionPlaythroughs,
         current_playthrough_offset() + offsets.MissionList
             + static_cast<ptrdiff_t>(offsetof(TArray<void>, count))}};
    mission_count.update(game_info);

    mission_count.suppress_changed();
    on_missions_changed();
}

void on_missions_changed(void) {
    runtime_print_message("Missions changed {} -> {}", mission_count.old(),
                          mission_count.current());

    auto mission_data_ptr = mission_count.pointer().dereference(game_info)
                            - offsetof(TArray<void>, count) + offsetof(TArray<void>, data);
    auto mission_data_list = asr_utils::read_address(game_info, mission_data_ptr);

    auto playthrough_mission_list_offset = current_playthrough_offset() + offsets.MissionList
                                           + static_cast<ptrdiff_t>(offsetof(TArray<void>, data));
    auto objectives_data_offset =
        offsets.ObjectivesProgress + static_cast<ptrdiff_t>(offsetof(TArray<void>, data));

    for (size_t i = 0; i < mission_count.current(); i++) {
        auto this_mission_offset = static_cast<ptrdiff_t>(offsets.MissionList_ElementSize * i);

        auto mission_name = read_from_gnames(asr_utils::read_mem<decltype(FName::index)>(
            game_info, asr_utils::read_address(game_info, mission_data_list + this_mission_offset
                                                              + offsets.MissionClass)
                           + offsetof(UObject, name.index)));

        if (match_objective_missions(mission_name, playthrough_mission_list_offset,
                                     this_mission_offset + objectives_data_offset)) {
            continue;
        }
        if (match_objective_set_missions(mission_name, playthrough_mission_list_offset,
                                         this_mission_offset + offsets.ActiveObjectiveSet)) {
            continue;
        }
        // All instant start missions are in Sanctuary, can exit early
        if (current_world != "Sanctuary3_P") {
            continue;
        }
        if (match_instant_start_missions(
                mission_name, mission_data_list + this_mission_offset + objectives_data_offset)) {
            continue;
        }
    }
}
