cmake_minimum_required(VERSION 3.14)
project(stevensSound VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# SDL2 is used for audio only — disable everything unrelated to audio
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_VIDEO OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_POWER OFF CACHE BOOL "" FORCE)
set(SDL_OPENGL OFF CACHE BOOL "" FORCE)
set(SDL_OPENGLES OFF CACHE BOOL "" FORCE)
set(SDL_WAYLAND_LIBDECOR OFF CACHE BOOL "" FORCE)
set(SDL_WAYLAND_LIBDECOR_SHARED OFF CACHE BOOL "" FORCE)

# Fetch SDL2 version 2.28.5 (fixes wmemcpy C++ bug and SDL_UDEV_DEVICE_TOUCHPAD udev regression in 2.24.x)
message(STATUS "Fetching SDL2 2.28.5...")
FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.28.5
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
set(SDL2MIXER_FLAC OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_OPUS OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MOD OFF CACHE BOOL "" FORCE)
set(SDL2MIXER_MIDI OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL2_mixer)

# Create a wrapper include directory with SDL2 subdirectory
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sdl_includes/SDL2)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${sdl2_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/sdl_includes/SDL2
)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${sdl2_mixer_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/sdl_includes/SDL2
)

# Create static library for stevensSound (compiled)
add_library(stevensSound STATIC stevensSound.cpp)
target_include_directories(stevensSound PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/sdl_includes>
)

# Link SDL2 and SDL2_mixer
if(TARGET SDL2_mixer)
    target_link_libraries(stevensSound PUBLIC SDL2::SDL2-static SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
    target_link_libraries(stevensSound PUBLIC SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static)
else()
    message(FATAL_ERROR "SDL2_mixer target not found")
endif()

target_compile_features(stevensSound PUBLIC cxx_std_20)

target_link_libraries(stevensSound PUBLIC stevensVectorLib stevensFileLib)

# Add thread support
find_package(Threads REQUIRED)
target_link_libraries(stevensSound PUBLIC Threads::Threads)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(stevensSound PUBLIC dl)
endif()

# If this is the main project (not included via add_subdirectory), build tests by default
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "stevensSound: Building as standalone project")
    option(STEVENSSOUND_BUILD_TESTS "Build tests" ON)
    option(STEVENSSOUND_BUILD_EXAMPLES "Build examples" ON)
    option(STEVENSSOUND_BUILD_BENCHMARKS "Build benchmarks" ON)
else()
    option(STEVENSSOUND_BUILD_TESTS "Build tests" OFF)
    option(STEVENSSOUND_BUILD_EXAMPLES "Build examples" OFF)
    option(STEVENSSOUND_BUILD_BENCHMARKS "Build benchmarks" OFF)
endif()

if(STEVENSSOUND_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(STEVENSSOUND_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(STEVENSSOUND_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
