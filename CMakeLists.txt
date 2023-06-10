cmake_minimum_required(VERSION 3.23)

project(wasm-autosplitters)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

add_subdirectory(asr-c)

add_subdirectory(bl3)
target_link_libraries(bl3 PRIVATE asr asr_asl asr_utils)
