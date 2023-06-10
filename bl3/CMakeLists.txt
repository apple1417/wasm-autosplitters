cmake_minimum_required(VERSION 3.23)

file(GLOB_RECURSE sources CONFIGURE_DEPENDS *.c *.cpp)
add_executable(bl3 ${sources})

set_target_properties(bl3 PROPERTIES
    INTERPROCEDURAL_OPTIMIZATION True
)

target_include_directories(bl3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_precompile_headers(bl3 PUBLIC pch.h)
