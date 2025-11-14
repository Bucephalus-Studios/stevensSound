# Tests for stevensSound library

include(GoogleTest)

# Create test executable
add_executable(stevensSound_tests
    test_main.cpp
    test_error_handler.cpp
    test_sound_loading.cpp
    test_sound_playback.cpp
)

target_link_libraries(stevensSound_tests
    PRIVATE
        stevensSound
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
)

# Discover tests
gtest_discover_tests(stevensSound_tests)

# Copy test assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test_assets
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
