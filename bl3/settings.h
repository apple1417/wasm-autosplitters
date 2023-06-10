#ifndef SETTINGS_H
#define SETTINGS_H

#include "pch.h"

struct Settings {
    bool start_echo;       // Picking up Claptrap's echo
    bool start_jackpot;    // Starting Jackpot DLC
    bool start_wedding;    // Starting Wedding DLC
    bool start_bounty;     // Starting Bounty DLC
    bool start_krieg;      // Starting Krieg DLC
    bool start_arms_race;  // Starting Arms Race DLC
    bool split_levels;     // Level transitions
    // Not supported by runtime yet
    // bool split_levels_dont_end;  // Unless doing so would end the run
    bool split_tyreen;   // Main Campaign ending cutscene
    bool split_jackpot;  // Jackpot DLC ending cutscene
    bool split_wedding;  // Wedding DLC ending cutscene
    bool split_bounty;   // Bounty DLC ending cutscene
    bool split_krieg;    // Krieg DLC ending cutscene
    bool use_char_time;  // Track character time, instead of loadless.
};

/**
 * @brief The main settings object
 */
extern Settings settings;

/**
 * @brief A map of mission class names which we start on pickup, to their setting value.
 */
extern const std::unordered_map<std::string, bool> STARTING_MISSIONS;

#endif /* SETTINGS_H */
