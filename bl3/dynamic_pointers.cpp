#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "sigscanning/gnames.h"
#include "sigscanning/gworld.h"
#include "sigscanning/localplayer.h"
#include "unreal_types.h"

namespace {

const constexpr auto STARTING_ECHO_MISSION = "Mission_Ep01_ChildrenOfTheVault_C";
const constexpr auto STARTING_ECHO_OBJECTIVE_IDX = 4;

const std::unordered_map<std::string, bool> STARTING_MISSIONS{
    {"Mission_DLC1_Ep01_MeetTimothy_C",
     user_settings_add_bool("start_jackpot", "Starting Jackpot DLC", true)},
    {"EP01_DLC2_C", user_settings_add_bool("start_wedding", "Starting Wedding DLC", true)},
    {"Mission_Ep01_WestlandWelcome_C",
     user_settings_add_bool("start_bounty", "Starting Bounty DLC", true)},
    {"ALI_EP01_C", user_settings_add_bool("start_krieg", "Starting Krieg DLC", true)},
    {"Mission_GearUp_Intro_C",
     user_settings_add_bool("start_arms_race", "Starting Arms Race DLC", true)}};

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

}  // namespace

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

        if (mission_name == STARTING_ECHO_MISSION) {
            starting_echo.pointer() = {
                base_localplayer,
                {
                    offsets.PlayerController,
                    offsets.PlayerMissionComponent,
                    offsets.MissionPlaythroughs,
                    playthrough_mission_list_offset,
                    this_mission_offset + objectives_data_offset,
                    static_cast<ptrdiff_t>(offsets.ObjectivesProgress_ElementSize
                                           * STARTING_ECHO_OBJECTIVE_IDX),

                }};
            starting_echo.update(game_info);
            // Don't want to start the run if you just load onto another character
            starting_echo.suppress_changed();

            continue;
        }

        if (!STARTING_MISSIONS.contains(mission_name) || !STARTING_MISSIONS.at(mission_name)
            || current_world.value() != "Sanctuary3_P") {
            continue;
        }

        /*
        Dectect picking up the missions we start on by looking for an incomplete first objective.

        This isn't perfect, but it's relatively simple and works for what we need it to. If we tried
        tracking what missions we had last update, we'd also need to track what character you've got
        selected, which also has side cases like deleting your char and making a new one with the
        same save game id, it just gets messy.

        Picking up a mission or loading into an ungeared save will have the first objective
        incomplete, so will get picked up by this, while it won't pick up loading into a save where
        you've already finished the dlc for the first time. The only side case is loading a save
        where you picked up one of the missions but didn't complete anything - i.e. bounty. There's
        no real way to tell the difference between this and loading an ungeared save though, it only
        happens in sancturary, and you can always just switch it off if it's really a problem.
        */

        auto first_objective = asr_utils::read_mem<int32_t>(
            game_info, asr_utils::read_address(game_info, mission_data_list + this_mission_offset
                                                              + offsets.ObjectivesProgress
                                                              + offsetof(TArray<void>, data))
                           + (0 * offsets.ObjectivesProgress_ElementSize));

        if (first_objective == 0) {
            runtime_print_message("Starting run due to picking up mission {}", mission_name);
            timer_start();
        }
    }
}
