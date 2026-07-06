cmake_minimum_required(VERSION 3.14)
project(stevensSound_examples)

# This subdirectory is only ever processed via add_subdirectory() from the
# root CMakeLists.txt, which defines the stevensSound target before reaching
# here (see STEVENSSOUND_BUILD_EXAMPLES).
if(NOT TARGET stevensSound)
    message(FATAL_ERROR "stevensSound target not found — build via the root CMakeLists.txt instead of this directory directly")
endif()

# Basic example
add_executable(basic basic_example.cpp)
target_link_libraries(basic PRIVATE stevensSound)

# Playlist example
add_executable(playlist playlist_example.cpp)
target_link_libraries(playlist PRIVATE stevensSound)

# Error handling example
add_executable(error_handling error_handling_example.cpp)
target_link_libraries(error_handling PRIVATE stevensSound)

# Copy example assets to build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
