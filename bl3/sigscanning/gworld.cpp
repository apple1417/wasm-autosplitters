#include "pch.h"
#include "sigscanning/gworld.h"
#include "sigscanning/gnames.h"
#include "unreal_types.h"

namespace {

const constinit asr_utils::Pattern<15> GWORLD_PATTERN{
    "4C 8D 0C 40"        // lea r9,[rax+rax*2]
    "48 8B 05 ????????"  // mov rax,[Borderlands3.exe+6175420] <----
    "4A 8D 0C C8"        // lea rcx,[rax+r9*8]
    ,
    7};

}

asr_utils::MemWatcher<decltype(FName::index)> gworld_name{
    {0, {0, offsetof(UObject, name.index)}},
    asr_utils::make_hex_variable<decltype(FName::index)>(DEBUG_VARIABLES ? "World FName Idx" : "")};
asr_utils::Variable<std::string> current_world{"World", "None"};

bool find_gworld(void) {
    auto gworld_ptr = asr_utils::sigscan(game_info, GWORLD_PATTERN);
    if (gworld_ptr == 0) {
        runtime_print_message("Failed to find GWorld, rejecting process");
        return false;
    }

    gworld_name.pointer().base = asr_utils::read_x86_offset(game_info, gworld_ptr);
    gworld_name.update(game_info);

    current_world = read_from_gnames(gworld_name.current());
    return true;
}
