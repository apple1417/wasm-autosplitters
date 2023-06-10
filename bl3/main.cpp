#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "settings.h"
#include "sigscanning/gnames.h"
#include "sigscanning/gworld.h"
#include "sigscanning/loading.h"
#include "sigscanning/localplayer.h"

namespace {

asr_utils::Variable<int32_t> char_time{"Character Time"};
asr_utils::Variable<std::string> in_game_world{DEBUG_VARIABLES ? "Last In Game World" : "None"};
asr_utils::Variable<uint32_t> save_quits{"SQs", 0};

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

        if (current_world == "MenuMap_P") {
            save_quits = save_quits.value() + 1;
        }
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
}

bool is_loading(ProcessId /*game*/) {
    return settings.use_char_time || loading.current() || current_world == "MenuMap_P";
}

Duration* game_time(ProcessId /*game*/) {
    if (!settings.use_char_time) {
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

    return false;
}
