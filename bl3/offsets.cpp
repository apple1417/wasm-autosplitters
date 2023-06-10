#include "pch.h"
#include "dynamic_pointers.h"
#include "offsets.h"
#include "sigscanning/gnames.h"
#include "sigscanning/localplayer.h"
#include "unreal_types.h"

DynamicOffsets offsets{};

namespace {

enum class OffsetProgress {
    NOT_STARTED,
    LOCALPLAYER_CLASS,
    PLAYER_CONTROLLER_CLASS,
    DONE,
    ERROR,
} progress = OffsetProgress::NOT_STARTED;

#ifdef __clang__
// Somehow, checking offsets in the header is fine, but here is not
// It doesn't like that the inherited classes aren't strictly POD
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

/**
 * @brief Iterates over all properties in a struct.
 *
 * @tparam T The type of the property. Can set to a more derived version to automatically read off
 *           it's extra fields.
 * @param ustruct The address of the struct to search through.
 * @param callback The callback to run with each property. Stops iterating early if it returns true.
 * @return True if stopped iterating early, false on error (including reaching the end of the list).
 */
template <typename T = UProperty, typename = std::enable_if<std::is_base_of_v<T, UProperty>>>
bool for_each_prop(const UStruct& ustruct, std::function<bool(const T& prop)> callback) {
    asr_utils::RemotePointer64<UProperty> prop_link = ustruct.property_link;

    while (prop_link.addr != 0) {
        T prop = prop_link.dereference<T>(game_info);

        if (callback(prop)) {
            return true;
        }

        prop_link = prop.property_link_next;
    }

    runtime_print_message("Ran out of properties while iterating!");
    progress = OffsetProgress::ERROR;
    return false;
}

}  // namespace

void reset_offsets(void) {
    progress = OffsetProgress::NOT_STARTED;

    offsets.PlayerController = 0;
    offsets.TimePlayedSeconds = 0;
    offsets.TimePlayedSecondsLoadedFromSaveGame = 0;
    offsets.PlayerMissionComponent = 0;
    offsets.CachedPlaythroughIndex = 0;
    offsets.MissionPlaythroughs = 0;
    offsets.MissionPlaythroughs_ElementSize = 0;
    offsets.MissionList = 0;
    offsets.MissionList_ElementSize = 0;
    offsets.MissionClass = 0;
    offsets.ActiveObjectiveSet = 0;
    offsets.ObjectivesProgress = 0;
    offsets.ObjectivesProgress_ElementSize = 0;
}
bool found_all_offsets(void) {
    return progress == OffsetProgress::DONE;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool find_offsets(void) {
    static asr_utils::DeepPointer localplayer_cls_ptr{};
    static asr_utils::DeepPointer pc_cls_ptr{};

    switch (progress) {
        case OffsetProgress::ERROR: {
            return false;
        }

        case OffsetProgress::NOT_STARTED: {
            progress = OffsetProgress::NOT_STARTED;

            localplayer_cls_ptr = {base_localplayer, {offsetof(UObject, cls), 0}};

            [[fallthrough]];
        }
        case OffsetProgress::LOCALPLAYER_CLASS: {
            progress = OffsetProgress::LOCALPLAYER_CLASS;

            // Wait for localplayer to be populated
            auto localplayer_cls = localplayer_cls_ptr.dereference(game_info);
            if (localplayer_cls == 0) {
                return false;
            }

            // Search for `localplayer.PlayerController` + extract class
            if (!for_each_prop<UProperty>(
                    read_mem<UClass>(game_info, localplayer_cls), [](auto prop) {
                        auto name = read_from_gnames(prop.name.index);
                        if (name == "PlayerController") {
                            offsets.PlayerController = prop.offset_internal;
                            return true;
                        }
                        return false;
                    })) {
                return false;
            }

            pc_cls_ptr = {base_localplayer, {offsets.PlayerController, offsetof(UObject, cls), 0}};

            [[fallthrough]];
        }
        case OffsetProgress::PLAYER_CONTROLLER_CLASS: {
            progress = OffsetProgress::PLAYER_CONTROLLER_CLASS;

            // Wait for player controller to be populated
            auto pc_cls = pc_cls_ptr.dereference(game_info);
            if (pc_cls == 0) {
                return false;
            }

            // Search for:
            //   `pc.TimePlayedSeconds`
            //   `pc.TimePlayedSecondsLoadedFromSaveGame`
            //   `pc.PlayerMissionComponent` + extract class
            size_t n_found_pc = 0;
            UClass mission_cls{};
            if (!for_each_prop<UObjectProperty>(
                    read_mem<UClass>(game_info, pc_cls), [&n_found_pc, &mission_cls](const UObjectProperty& prop) {
                        auto name = read_from_gnames(prop.name.index);
                        if (name == "TimePlayedSeconds") {
                            offsets.TimePlayedSeconds = prop.offset_internal;
                            n_found_pc++;
                        } else if (name == "TimePlayedSecondsLoadedFromSaveGame") {
                            offsets.TimePlayedSecondsLoadedFromSaveGame = prop.offset_internal;
                            n_found_pc++;
                        } else if (name == "PlayerMissionComponent") {
                            offsets.PlayerMissionComponent = prop.offset_internal;
                            n_found_pc++;

                            mission_cls = prop.property_class.dereference(game_info);
                        }

                        return n_found_pc >= 3;
                    })) {
                return false;
            }

            // Search for:
            //   `mission.CachedPlaythroughIndex`
            //   `mission.MissionPlaythroughs` + extract struct
            //   `sizeof(mission.MissionPlaythroughs[0])`
            size_t n_found_mission = 0;
            UScriptStruct playthroughs_entry{};
            if (!for_each_prop<UArrayProperty>(mission_cls, [&n_found_mission, &playthroughs_entry](
                                                                const UArrayProperty& prop) {
                    auto name = read_from_gnames(prop.name.index);
                    if (name == "CachedPlaythroughIndex") {
                        offsets.CachedPlaythroughIndex = prop.offset_internal;
                        n_found_mission++;
                    } else if (name == "MissionPlaythroughs") {
                        offsets.MissionPlaythroughs = prop.offset_internal;
                        n_found_mission++;

                        auto playthroughs_entry_prop =
                            prop.inner.dereference<UStructProperty>(game_info);
                        offsets.MissionPlaythroughs_ElementSize =
                            playthroughs_entry_prop.element_size;
                        playthroughs_entry = playthroughs_entry_prop.inner.dereference(game_info);
                    }

                    return n_found_mission >= 2;
                })) {
                return false;
            }

            // Search for:
            //   `playthrough.MissionList` + extract struct
            //   `sizeof(playthrough.MissionList[0])`
            UScriptStruct missionlist_entry{};
            if (!for_each_prop<UArrayProperty>(
                    playthroughs_entry, [&missionlist_entry](const UArrayProperty& prop) {
                        if (read_from_gnames(prop.name.index) == "MissionList") {
                            offsets.MissionList = prop.offset_internal;

                            auto missionlist_entry_prop =
                                prop.inner.dereference<UStructProperty>(game_info);
                            offsets.MissionList_ElementSize = missionlist_entry_prop.element_size;
                            missionlist_entry = missionlist_entry_prop.inner.dereference(game_info);

                            return true;
                        }
                        return false;
                    })) {
                return false;
            }

            // Search for:
            //   `entry.MissionClass`
            //   `entry.ActiveObjectiveSet`
            //   `entry.ObjectivesProgress`
            //   `sizeof(entry.ObjectivesProgress[0])`
            size_t n_found_entry = 0;
            if (!for_each_prop<UArrayProperty>(
                    missionlist_entry, [&n_found_entry](const UArrayProperty& prop) {
                        auto name = read_from_gnames(prop.name.index);
                        if (name == "MissionClass") {
                            offsets.MissionClass = prop.offset_internal;
                            n_found_entry++;
                        } else if (name == "ActiveObjectiveSet") {
                            offsets.ActiveObjectiveSet = prop.offset_internal;
                            n_found_entry++;
                        } else if (name == "ObjectivesProgress") {
                            offsets.ObjectivesProgress = prop.offset_internal;
                            n_found_entry++;

                             offsets.ObjectivesProgress_ElementSize = prop.inner.dereference(game_info).element_size;
                        }

                        return n_found_entry >= 3;
                    })) {
                return false;
            }

            // We have all the offsets, init the pointers
            init_dynamic_pointers();

            [[fallthrough]];
        }
        case OffsetProgress::DONE: {
            progress = OffsetProgress::DONE;

            return true;
        }
    }
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
