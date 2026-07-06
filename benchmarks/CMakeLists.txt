cmake_minimum_required(VERSION 3.14)
project(stevensSound_benchmarks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# This subdirectory is only ever processed via add_subdirectory() from the
# root CMakeLists.txt, which defines the stevensSound target before reaching
# here (see STEVENSSOUND_BUILD_BENCHMARKS).
if(NOT TARGET stevensSound)
    message(FATAL_ERROR "stevensSound target not found — build via the root CMakeLists.txt instead of this directory directly")
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
