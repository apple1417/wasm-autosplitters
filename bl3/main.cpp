#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "sigscanning/gnames.h"
#include "sigscanning/gworld.h"
#include "sigscanning/loading.h"
#include "sigscanning/localplayer.h"

namespace {

asr_utils::Variable<int32_t> char_time{"Char Time"};

}  // namespace

asr_utils::ProcessInfo game_info{};

const MatchableExecutableName MATCHABLE_EXECUTABLES[] = {
    MATCH_EXECUTABLE("Borderlands3.exe"),
    MATCH_EXECUTABLE_TRUNC15("Borderlands3.exe"),
    END_MATCHABLE_EXECUTABLES(),
};

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
    gworld_name.update(game_info);
    loading.update(game_info);

    if (gworld_name.changed() && gworld_name.current() != 0) {
        auto new_world = read_from_gnames(gworld_name.current());
        runtime_print_message("World changed {} -> {}", current_world, new_world);

        current_world = new_world;
    }

    if (!find_offsets()) {
        return true;
    }

    // These pointers depend on the dynamic offsets, no sense updating them earlier
    playthrough.update(game_info);
    time_played.update(game_info);
    time_played_save_game.update(game_info);
    mission_count.update(game_info);
    starting_echo.update(game_info);

    char_time = time_played.current() + time_played_save_game.current();

    if (playthrough.changed()) {
        on_playthrough_changed();
    }
    if (mission_count.changed()) {
        on_missions_changed();
    }

    return true;
}

bool is_loading(ProcessId /*game*/) {
    return loading.current() || current_world.value() == "MenuMap_P";
}

bool start(ProcessId /* game */) {
    if (!found_all_offsets()) {
        return false;
    }

    if (starting_echo.changed() && starting_echo.current() == 1
        && current_world.value() == "Recruitment_P") {
        runtime_print_message("Starting due to collecting echo.");
        return true;
    }

    return false;
}
