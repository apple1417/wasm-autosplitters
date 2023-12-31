#ifndef PCH_H
#define PCH_H

#include <asl.h>
#include <asr.h>

#ifdef __cplusplus

#include <asr_utils.h>
#include <asr_utils/pch.h>

#include <chrono>
#include <cstddef>
#include <unordered_map>

#if defined(NDEBUG)
#define DEBUG_VARIABLES false
#else
#define DEBUG_VARIABLES true
#endif

/**
 * @brief The currently attached game's process info.
 * @note This is defined in main.cpp, but we put it in the PCH so it's easily available everywhere.
 */
extern asr_utils::ProcessInfo game_info;

#endif

#endif /* PCH_H */
