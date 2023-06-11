#ifndef UNREAL_TYPES_H
#define UNREAL_TYPES_H

#include "pch.h"

// TODO: read these dynamically
//       idk how to do enums yet
enum class EMissionStatus : uint8_t {
    MS_NotStarted = 0,
    MS_Active = 1,
    MS_Complete = 2,
    MS_Failed = 3,
    MS_Unknown = 4,
    MS_MAX,
};

template <typename T>
struct TArray {
    asr_utils::RemotePointer64<T> data;
    int32_t count;
    int32_t max;
};

struct FName {
    int32_t index;
    int32_t number;
};

struct UClass;

struct UObject {
    asr_utils::RemotePointer64<void> vftable;
    int32_t object_flags;
    int32_t internal_idx;
    asr_utils::RemotePointer64<UClass> cls;
    FName name;
    asr_utils::RemotePointer64<UObject> outer;
};

struct UField : public UObject {
    asr_utils::RemotePointer64<UField> next;
};

struct UProperty;

struct UStruct : public UField {
    asr_utils::RemotePointer64<UStruct> super_field;
    asr_utils::RemotePointer64<UField> children;
    int32_t property_size;
    int32_t min_alignment;
    TArray<uint8_t> script;
    asr_utils::RemotePointer64<UProperty> property_link;
    asr_utils::RemotePointer64<UProperty> ref_link;
    asr_utils::RemotePointer64<UProperty> destructor_link;
    asr_utils::RemotePointer64<UProperty> post_construct_link;
    TArray<asr_utils::RemotePointer64<UObject>> script_object_references;
};

struct UClass : public UStruct {};

struct UScriptStruct : public UStruct {
    uint32_t struct_flags;
};

struct UProperty : public UField {
    int32_t array_dim;
    int32_t element_size;
    uint64_t property_flags;
    uint16_t rep_index;
    uint8_t blueprint_replication_condition;
    int32_t offset_internal;
    FName rep_notify_func;
    asr_utils::RemotePointer64<UProperty> property_link_next;
    asr_utils::RemotePointer64<UProperty> next_ref;
    asr_utils::RemotePointer64<UProperty> destructor_link_next;
    asr_utils::RemotePointer64<UProperty> post_construct_link_next;
};

struct UObjectProperty : public UProperty {
    asr_utils::RemotePointer64<UClass> property_class;
};

struct UClassProperty : public UObjectProperty {
    asr_utils::RemotePointer64<UClass> meta_cls;
};

struct UArrayProperty : public UProperty {
    asr_utils::RemotePointer64<UProperty> inner;
};

struct UStructProperty : public UProperty {
    asr_utils::RemotePointer64<UScriptStruct> inner;
};

struct FNameEntry {
    static constexpr auto NAME_SIZE = 1024;

    int32_t index;
    uint8_t unknown[0x4];
    asr_utils::RemotePointer64<FNameEntry> hash_next;
    union {
        char ansi_name[NAME_SIZE];
        wchar_t wide_name[NAME_SIZE];
    };
};

struct GNames {
    static constexpr auto MAX_TOTAL_ELEMENTS = 4 * 1024 * 1024;
    static constexpr auto ELEMENTS_PER_CHUNK = 16384;
    static constexpr auto CHUNK_TABLE_SIZE =
        (MAX_TOTAL_ELEMENTS + ELEMENTS_PER_CHUNK - 1) / ELEMENTS_PER_CHUNK;

    asr_utils::RemotePointer64<asr_utils::RemotePointer64<FNameEntry>> objects[CHUNK_TABLE_SIZE];
    int32_t count;
    int32_t chunks_count;
};

// NOLINTBEGIN(readability-magic-numbers)

// Since inheritance kills POD assumptions, make sure everything's still where we expect it to be
static_assert(offsetof(TArray<void>, data) == 0x0);
static_assert(offsetof(TArray<void>, count) == 0x8);
static_assert(offsetof(FName, index) == 0x0);
static_assert(offsetof(UObject, cls) == 0x10);
static_assert(offsetof(UObject, name) == 0x18);
static_assert(offsetof(UField, next) == 0x28);
static_assert(offsetof(UStruct, property_link) == 0x58);
static_assert(offsetof(UProperty, element_size) == 0x34);
static_assert(offsetof(UProperty, offset_internal) == 0x44);
static_assert(offsetof(UObjectProperty, property_class) == 0x70);
static_assert(offsetof(UClassProperty, property_class) == 0x70);
static_assert(offsetof(UStructProperty, inner) == 0x70);
static_assert(offsetof(FNameEntry, ansi_name) == 0x10);
static_assert(offsetof(GNames, objects) == 0x0);

// NOLINTEND(readability-magic-numbers)

#endif /* UNREAL_TYPES_H */
