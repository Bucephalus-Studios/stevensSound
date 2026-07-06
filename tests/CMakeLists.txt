cmake_minimum_required(VERSION 3.14)
project(stevensSound_tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# This subdirectory is only ever processed via add_subdirectory() from the
# root CMakeLists.txt, which defines the stevensSound target before reaching
# here (see STEVENSSOUND_BUILD_TESTS).
if(NOT TARGET stevensSound)
    message(FATAL_ERROR "stevensSound target not found — build via the root CMakeLists.txt instead of this directory directly")
endif()

# Fetch GoogleTest (always needed for tests)
message(STATUS "Fetching GoogleTest...")
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)
enable_testing()

# Create test executable
add_executable(tests
    test_main.cpp
    test_error_handler.cpp
    test_sound_loading.cpp
    test_sound_playback.cpp
)

target_link_libraries(tests
    PRIVATE
        stevensSound
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

# Discover tests
gtest_discover_tests(tests)

# Copy test assets to build directory if they exist
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/../test_assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/../test_assets
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
