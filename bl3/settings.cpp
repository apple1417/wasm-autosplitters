#include "pch.h"

#include "settings.h"

namespace {

/**
 * @brief Creates the settings struct, making sure to order all fields correctly.
 *
 * @return The settings struct.
 */
Settings init_settings(void) {
    user_settings_add_bool("start_header", "Start the run on ...", false);
    auto start_echo = user_settings_add_bool("start_echo", "- Picking up Claptrap's echo", true);
    auto start_jackpot = user_settings_add_bool("start_jackpot", "- Starting Jackpot DLC", true);
    auto start_wedding = user_settings_add_bool("start_wedding", "- Starting Wedding DLC", true);
    auto start_bounty = user_settings_add_bool("start_bounty", "- Starting Bounty DLC", true);
    auto start_krieg = user_settings_add_bool("start_krieg", "- Starting Krieg DLC", true);
    auto start_arms_race =
        user_settings_add_bool("start_arms_race", "- Starting Arms Race DLC", true);

    user_settings_add_bool("split_header", "Split on ...", false);
    auto split_levels = user_settings_add_bool("split_levels", "- Level transitions", false);
    /*
    auto split_levels_dont_end = user_settings_add_bool(
        "split_levels_dont_end", "- - Unless doing so would end the run", true);
    */
    auto split_any_mission =
        user_settings_add_bool("split_any_mission", "- Mission completions", false);
    auto split_tyreen =
        user_settings_add_bool("split_tyreen", "- Main Campaign ending cutscene", true);
    auto split_jackpot =
        user_settings_add_bool("split_jackpot", "- Jackpot DLC ending cutscene", true);
    auto split_wedding =
        user_settings_add_bool("split_wedding", "- Wedding DLC ending cutscene", true);
    auto split_bounty =
        user_settings_add_bool("split_bounty", "- Bounty DLC ending cutscene", true);
    auto split_krieg = user_settings_add_bool("split_krieg", "- Krieg DLC ending cutscene", true);

    auto use_char_time = user_settings_add_bool(
        "use_char_time", "Track character time, instead of loadless.", false);

    return {
        start_echo,   start_jackpot,   start_wedding, start_bounty,
        start_krieg,  start_arms_race, split_levels,  /*split_levels_dont_end,*/ split_any_mission,
        split_tyreen, split_jackpot,   split_wedding, split_bounty,
        split_krieg,  use_char_time,
    };
}

}  // namespace

Settings settings = init_settings();
