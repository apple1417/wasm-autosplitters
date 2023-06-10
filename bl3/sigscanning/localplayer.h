#ifndef LOCALPLAYER_H
#define LOCALPLAYER_H

#include "pch.h"

/**
 * @brief The address holding a pointer to the local player.
 */
extern asr_utils::HexVariable<Address> base_localplayer;

/**
 * @brief Tries to find the local player base address on attaching to a game.
 *
 * @return True if successful.
 */
bool find_localplayer(void);

#endif /* LOCALPLAYER_H */
