#include "pch.h"
#include "sigscanning/gnames.h"
#include "unreal_types.h"

namespace {

const constinit asr_utils::Pattern<46> GNAMES_PATTERN{
    "E8 ????????"        // call Borderlands3.exe+3DB68AC
    "48 ?? ??"           // mov rax,rbx
    "48 89 1D ????????"  // mov [Borderlands3.exe+69426C8],rbx   <----
    "48 8B 5C 24 ??"     // mov rbx,[rsp+20]
    "48 83 C4 28"        // add rsp,28
    "C3"                 // ret
    "?? DB"              // xor ebx,ebx
    "48 89 1D ????????"  // mov [Borderlands3.exe+69426C8],rbx
    "?? ??"              // mov eax,ebx
    "48 8B 5C 24 ??"     // mov rbx,[rsp+20]
    "48 83 C4 ??"        // add rsp,28
    "C3"                 // ret
    ,
    11};

asr_utils::HexVariable<uint64_t> gnames{DEBUG_VARIABLES ? "GNames" : ""};
std::unordered_map<decltype(FName::index), std::string> gnames_cache{};

}  // namespace

bool find_gnames(void) {
    auto gnames_ptr = asr_utils::sigscan(game_info, GNAMES_PATTERN);
    if (gnames_ptr == 0) {
        runtime_print_message("Failed to find GNames, rejecting process");
        return false;
    }

    gnames = asr_utils::read_x86_offset(game_info, gnames_ptr);
    gnames_cache.clear();
    return true;
}

const std::string& read_from_gnames(decltype(FName::index) idx) {
    if (!gnames_cache.contains(idx)) {
        // GNames uses a chunked array, so index becomes two offsets
        auto outer_offset = static_cast<ptrdiff_t>(
            (idx / GNames::ELEMENTS_PER_CHUNK)
            * sizeof(asr_utils::RemotePointer64<asr_utils::RemotePointer64<FNameEntry>>));
        auto inner_offset = static_cast<ptrdiff_t>(
            (idx % GNames::ELEMENTS_PER_CHUNK) * sizeof(asr_utils::RemotePointer64<FNameEntry>));

        asr_utils::DeepPointer ptr{gnames + offsetof(GNames, objects),
                                   {outer_offset, inner_offset, offsetof(FNameEntry, ansi_name)}};

        auto str = read_string(game_info, ptr.dereference(game_info));
        gnames_cache.emplace(idx, std::move(str));
    }

    return gnames_cache[idx];
}
