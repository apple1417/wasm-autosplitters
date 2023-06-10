#ifndef GNAMES_H
#define GNAMES_H

#include "pch.h"
#include "unreal_types.h"

/**
 * @brief Tries to find gnames on attaching to a game.
 *
 * @return True if successful.
 */
bool find_gnames(void);

/**
 * @brief Reads a value out of GNames.
 *
 * @param idx The names index to read.
 * @return A reference to the name. Only valid until the next time the game is hooked.
 */
const std::string& read_from_gnames(decltype(FName::index) idx);

#endif /* GNAMES_H */
