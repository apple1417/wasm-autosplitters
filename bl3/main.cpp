#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "settings.h"
#include "sigscanning/gnames.h"
#include "sigscanning/gworld.h"
#include "sigscanning/loading.h"
#include "sigscanning/localplayer.h"
#include "unreal_types.h"

using namespace std::chrono_literals;

namespace {

asr_utils::Variable<int32_t> char_time{"Character Time"};

asr_utils::Variable<uint32_t> save_quits{"SQs", 0};
asr_utils::Variable<std::string> in_game_world{DEBUG_VARIABLES ? "Last In Game World" : "", "None"};

struct ObjectiveSetSplitData {
    asr_utils::MemWatcher<int32_t>& watcher;
    asr_utils::Variable<std::string> var;
    const bool Settings::*setting;
    const std::string final_objective_set;
    const std::chrono::milliseconds split_delay;
};
// NOLINTNEXTLINE(readability-identifier-naming)
std::vector<ObjectiveSetSplitData> SPLIT_OBJECTIVE_SET_DATA;

using clk = std::chrono::steady_clock;
clk::time_point deferred_split_time = clk::time_point::max();

}  // namespace

asr_utils::ProcessInfo game_info{};

const MatchableExecutableName MATCHABLE_EXECUTABLES[] = {
    MATCH_EXECUTABLE("Borderlands3.exe"),
    MATCH_EXECUTABLE_TRUNC15("Borderlands3.exe"),
    END_MATCHABLE_EXECUTABLES(),
};

void startup(void) {
    // Need to initialize this here to ensure the references to the mem watchers are valid
    // NOLINTBEGIN(readability-magic-numbers)
    SPLIT_OBJECTIVE_SET_DATA = std::vector<ObjectiveSetSplitData>{
        {tyreen_cutscene_objective_set,
         {DEBUG_VARIABLES ? "Tyreen Objective Set" : "", "None"},
         &Settings::split_tyreen,
         "Set_TyreenDeadCine_ObjectiveSet",
         2s},
        {jackpot_cutscene_objective_set,
         {DEBUG_VARIABLES ? "Jackpot Objective Set" : "", "None"},
         &Settings::split_jackpot,
         "Set_FinalCinematic_ObjectiveSet",
         1s},
        {wedding_cutscene_objective_set,
         {DEBUG_VARIABLES ? "Wedding Objective Set" : "", "None"},
         &Settings::split_wedding,
         "Set_FinalCredits_ObjectiveSet",
         1s},
        {bounty_cutscene_objective_set,
         {DEBUG_VARIABLES ? "Bounty Objective Set" : "", "None"},
         &Settings::split_bounty,
         "SET_EndCredits_ObjectiveSet",
         100ms},
        {krieg_cutscene_objective_set,
         {DEBUG_VARIABLES ? "Krieg Objective Set" : "", "None"},
         &Settings::split_krieg,
         "SET_OutroCIN_ObjectiveSet",
         1s},
    };
    // NOLINTEND(readability-magic-numbers)
}

bool on_launch(ProcessId game, const MatchableExecutableName* /*name*/) {
    game_info = {game, "Borderlands3.exe"};

    // Make sure we find gnames first
    if (!find_gnames()) {
        return false;
    }
    // Short circuit the rest
    if (!(find_gworld() && find_loading() && find_localplayer())) {
        return false;
    }

    reset_offsets();

    return true;
}

void on_exit(void) {
    timer_pause_game_time();
}

bool update(ProcessId /*game*/) {
    if (deferred_split_time < clk::now()) {
        timer_split();
        runtime_print_message("Splitting as reached deferred split time.");
        deferred_split_time = clk::time_point::max();
    }

    gworld_name.update(game_info);
    loading.update(game_info);

    if (gworld_name.changed() && gworld_name.current() != 0) {
        auto new_world = read_from_gnames(gworld_name.current());
        runtime_print_message("World changed {} -> {}", current_world, new_world);

        current_world = new_world;

        if (current_world == "MenuMap_P") {
            save_quits = save_quits.value() + 1;
        }
    }

    if (!find_offsets()) {
        return true;
    }

    // These pointers depend on the dynamic offsets, no sense updating them earlier
    for (auto watcher :
         {&playthrough, &time_played, &time_played_save_game, &mission_count, &starting_echo}) {
        watcher->update(game_info);
    }

    for (auto& [watcher, var, _, _2, _3] : SPLIT_OBJECTIVE_SET_DATA) {
        watcher.update(game_info);
        if (watcher.changed()) {
            var = read_from_gnames(watcher.current());
        }
    }

    // Use an iterator so we can erase during the loop
    for (auto it = incomplete_missions.begin(); it != incomplete_missions.end();) {
        it->update(game_info);

        if (it->changed()
            && static_cast<EMissionStatus>(it->current()) == EMissionStatus::MS_Complete) {
            // Handling this here also lets us split multiple times per tick... if we ever need that
            runtime_print_message("Splitting due to mission completion");
            timer_split();

            it = incomplete_missions.erase(it);
            incomplete_mission_count = incomplete_missions.size();
        } else {
            it++;
        }
    }

    char_time = time_played.current() + time_played_save_game.current();

    if (playthrough.changed()) {
        on_playthrough_changed();
    }
    if (mission_count.changed()) {
        on_missions_changed();
    }

    return true;
}

bool start(ProcessId /* game */) {
    if (!found_all_offsets()) {
        return false;
    }

    if (settings.start_echo && starting_echo.changed() && starting_echo.current() == 1
        && current_world == "Recruitment_P") {
        runtime_print_message("Starting due to collecting echo.");
        return true;
    }

    return false;
}

void on_start(void) {
    in_game_world = "None";
    save_quits = 0;
    deferred_split_time = clk::time_point::max();
}

bool is_loading(ProcessId /*game*/) {
    return settings.use_char_time || loading.current() || current_world == "MenuMap_P";
}

Duration* game_time(ProcessId /*game*/) {
    if (!settings.use_char_time || char_time == 0) {
        return nullptr;
    }

    static Duration duration{0, 0};
    duration.secs = char_time;
    return &duration;
}

bool split(ProcessId /*game*/) {
    if (settings.split_levels && current_world != "None" && in_game_world != "None"
        && current_world != "MenuMap_P" && current_world.value() != in_game_world.value()) {
        in_game_world = current_world.value();
        runtime_print_message("Splitting due to level transition");
        return true;
    }

    for (auto& [watcher, var, setting, expected, delay] : SPLIT_OBJECTIVE_SET_DATA) {
        if (settings.*setting && watcher.changed() && watcher.old() != 0 && var == expected) {
            runtime_print_message("Scheduling split due to hitting objective set {}", expected);
            deferred_split_time = clk::now() + delay;
            return false;
        }
    }

    return false;
}
