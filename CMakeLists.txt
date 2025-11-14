cmake_minimum_required(VERSION 3.14)
project(stevensSound VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(STEVENSSOUND_BUILD_TESTS "Build tests" ON)
option(STEVENSSOUND_BUILD_BENCHMARKS "Build benchmarks" ON)
option(STEVENSSOUND_BUILD_EXAMPLES "Build examples" ON)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

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

# Make SDL2 available first
FetchContent_MakeAvailable(SDL2)

# Set SDL2 paths for SDL2_mixer to find
set(SDL2_DIR "${sdl2_BINARY_DIR}")
set(SDL2_INCLUDE_DIR "${sdl2_SOURCE_DIR}/include")
set(SDL2_LIBRARY SDL2::SDL2-static)

# Fetch SDL2_mixer version 2.6.3 (more stable with our build setup)
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

# Make SDL2_mixer available
FetchContent_MakeAvailable(SDL2_mixer)

# Create interface library for stevensSound (header-only)
add_library(stevensSound INTERFACE)
target_include_directories(stevensSound INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Link SDL2 and SDL2_mixer
# SDL2_mixer 2.8.x uses SDL2_mixer as target name, not SDL2_mixer::SDL2_mixer-static
if(TARGET SDL2_mixer)
    target_link_libraries(stevensSound INTERFACE SDL2::SDL2-static SDL2_mixer)
elseif(TARGET SDL2_mixer::SDL2_mixer-static)
    target_link_libraries(stevensSound INTERFACE SDL2::SDL2-static SDL2_mixer::SDL2_mixer-static)
else()
    message(FATAL_ERROR "SDL2_mixer target not found")
endif()

# Set compile features
target_compile_features(stevensSound INTERFACE cxx_std_20)

# Add thread support
find_package(Threads REQUIRED)
target_link_libraries(stevensSound INTERFACE Threads::Threads)

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(stevensSound INTERFACE dl)
endif()

# Fetch GoogleTest if building tests
if(STEVENSSOUND_BUILD_TESTS)
    message(STATUS "Fetching GoogleTest...")
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(tests)
endif()

# Fetch Google Benchmark if building benchmarks
if(STEVENSSOUND_BUILD_BENCHMARKS)
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

    add_subdirectory(benchmarks)
endif()

# Build examples
if(STEVENSSOUND_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation rules
include(GNUInstallDirs)

# Since stevensSound is an interface library that links to FetchContent dependencies,
# we don't export the targets (SDL2/SDL2_mixer are bundled)
install(TARGETS stevensSound
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(FILES stevensSound.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stevensSound
)

install(DIRECTORY classes/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stevensSound/classes
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(DIRECTORY libraries/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/stevensSound/libraries
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Create package config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/stevensSoundConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Print configuration summary
message(STATUS "")
message(STATUS "stevensSound Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build tests: ${STEVENSSOUND_BUILD_TESTS}")
message(STATUS "  Build benchmarks: ${STEVENSSOUND_BUILD_BENCHMARKS}")
message(STATUS "  Build examples: ${STEVENSSOUND_BUILD_EXAMPLES}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "")
