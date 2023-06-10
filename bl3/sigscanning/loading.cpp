#include "pch.h"
#include "sigscanning/loading.h"

namespace {

// Before OAK-PATCHDIESEL0-280
const constinit asr_utils::Pattern<16> LOADING_PATTERN_OLD{
    "C7 44 24 28 0C000010"  // mov [rsp+28],1000000C
    "C7 44 24 20 D0010000"  // mov [rsp+20],000001D0
    ,
    -119};
const constinit asr_utils::Pattern<16> LOADING_PATTERN_NEW{
    "C7 44 24 28 0C000010"  // mov [rsp+28],1000000C
    "C7 44 24 20 F0010000"  // mov [rsp+20],000001F0
    ,
    -119};
const constexpr auto LOADING_OFFSET_1 = 0xF8;
const constexpr auto LOADING_OFFSET_2_OLD = 0x9DC;
const constexpr auto LOADING_OFFSET_2_NEW = 0xA7C;

}  // namespace

asr_utils::MemWatcher<bool> loading{
    {},
    asr_utils::make_variable<bool>(DEBUG_VARIABLES ? "Is Loading" : "")};

bool find_loading(void) {
    bool using_old = true;
    auto loading_ptr = asr_utils::sigscan(game_info, LOADING_PATTERN_OLD);
    if (loading_ptr == 0) {
        using_old = false;
        loading_ptr = asr_utils::sigscan(game_info, LOADING_PATTERN_NEW);
        if (loading_ptr == 0) {
            runtime_print_message("Failed to find loading pointer, rejecting process");
            return false;
        }
    }

    loading.pointer() = {
        asr_utils::read_x86_offset(game_info, loading_ptr),
        {LOADING_OFFSET_1, using_old ? LOADING_OFFSET_2_OLD : LOADING_OFFSET_2_NEW}};
    loading.update(game_info);

    return true;
}
