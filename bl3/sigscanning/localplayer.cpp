#include "pch.h"
#include "sigscanning/localplayer.h"

namespace {

const constinit asr_utils::Pattern<40> LOCAL_PLAYER_PATTERN{
    "88 1D ????????"     // mov [Borderlands3.exe+6A5A794],bl { (0) }
    "E8 ????????"        // call Borderlands3.exe+3DB17A4
    "48 8D 0D ????????"  // lea rcx,[Borderlands3.exe+6A5A798] { (-2147481615) }
    "E8 ????????"        // call Borderlands3.exe+3DB1974
    "48 8B 5C 24 20"     // mov rbx,[rsp+20]
    "48 8D 05 ????????"  // lea rax,[Borderlands3.exe+6A5A6A0] { (0) }   <----
    "48 83 C4 28"        // add rsp,28 { 40 }
    "C3"                 // ret
    ,
    31};

constexpr const auto LOCALPLAYER_OFFSET = (0x8 * 0x1A);

}  // namespace

asr_utils::HexVariable<Address> base_localplayer{DEBUG_VARIABLES ? "LocalPlayer" : ""};

bool find_localplayer(void) {
    auto localplayer_ptr = asr_utils::sigscan(game_info, LOCAL_PLAYER_PATTERN);
    if (localplayer_ptr == 0) {
        runtime_print_message("Failed to find LocalPlayer, rejecting process");
        return false;
    }

    base_localplayer = asr_utils::read_x86_offset(game_info, localplayer_ptr) + LOCALPLAYER_OFFSET;

    return true;
}
