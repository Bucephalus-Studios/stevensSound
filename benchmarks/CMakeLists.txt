cmake_minimum_required(VERSION 3.14)
project(stevensSound_benchmarks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include FetchContent for dependency management
include(FetchContent)

# Check if stevensSound target already exists (included from root CMakeLists.txt)
if(NOT TARGET stevensSound)
    message(STATUS "stevensSound target not found - setting up dependencies for standalone build")

# Disable building unnecessary SDL2 components
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)

# Fetch SDL2 version 2.0.22
message(STATUS "Fetching SDL2 2.0.22...")
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.0.22
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL2)

# Set SDL2 paths for SDL2_mixer to find
set(SDL2_DIR "${sdl2_BINARY_DIR}")
set(SDL2_INCLUDE_DIR "${sdl2_SOURCE_DIR}/include")
set(SDL2_LIBRARY SDL2::SDL2-static)

# Fetch SDL2_mixer version 2.6.3
message(STATUS "Fetching SDL2_mixer 2.6.3...")
FetchContent_Declare(
    SDL2_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-2.6.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

# Configure SDL2_mixer options
set(SDL2MIXER_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_DEPS_SHARED OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_VENDORED ON CACHE BOOL "" FORCE)
set(SDL2MIXER_WAVE ON CACHE BOOL "" FORCE)
set(SDL2MIXER_MP3 ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OGG ON CACHE BOOL "" FORCE)
set(SDL2MIXER_FLAC ON CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2_mixer)

# Fetch SoundTouch library for pitch shifting
message(STATUS "Fetching SoundTouch library...")
FetchContent_Declare(
    SoundTouch
    GIT_REPOSITORY https://gitlab.com/soundtouch/soundtouch.git
    GIT_TAG 2.3.1
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(SOUNDTOUCH_DLL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SoundTouch)

# Create interface library for stevensSound (header-only)
add_library(stevensSound INTERFACE)
target_include_directories(stevensSound INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Link SDL2 and SDL2_mixer
if(TARGET SDL2_mixer)
    target_link_libraries(stevensSound INTERFACE SDL2::SDL2-static SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
    target_link_libraries(stevensSound INTERFACE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static)
else()
    message(FATAL_ERROR "SDL2_mixer target not found")
endif()

# Link SoundTouch
if(TARGET SoundTouch)
    target_link_libraries(stevensSound INTERFACE SoundTouch)
elseif(TARGET SoundTouch::SoundTouch)
    target_link_libraries(stevensSound INTERFACE SoundTouch::SoundTouch)
else()
    message(FATAL_ERROR "SoundTouch target not found")
endif()

target_compile_features(stevensSound INTERFACE cxx_std_20)

# Add thread support
find_package(Threads REQUIRED)
target_link_libraries(stevensSound INTERFACE Threads::Threads)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(stevensSound INTERFACE dl)
endif()
else()
    message(STATUS "Using existing stevensSound target from parent")
endif()

# Fetch Google Benchmark
message(STATUS "Fetching Google Benchmark...")
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
    GIT_SHALLOW TRUE
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Create benchmark executable
add_executable(benchmarks
    benchmark_main.cpp
    benchmark_sound_loading.cpp
    benchmark_sound_playback.cpp
)

target_link_libraries(benchmarks
    PRIVATE
        stevensSound
        benchmark::benchmark
        benchmark::benchmark_main
)

# Copy test assets to build directory if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../test_assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../test_assets
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
