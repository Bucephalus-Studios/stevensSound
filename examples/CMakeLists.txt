# Examples for stevensSound library

# Basic example
add_executable(basic_example basic_example.cpp)
target_link_libraries(basic_example PRIVATE stevensSound)

# Playlist example
add_executable(playlist_example playlist_example.cpp)
target_link_libraries(playlist_example PRIVATE stevensSound)

# Error handling example
add_executable(error_handling_example error_handling_example.cpp)
target_link_libraries(error_handling_example PRIVATE stevensSound)

# RAII example
add_executable(raii_example raii_example.cpp)
target_link_libraries(raii_example PRIVATE stevensSound)

# Pitch modulation example
add_executable(pitch_modulation_example pitch_modulation_example.cpp)
target_link_libraries(pitch_modulation_example PRIVATE stevensSound)

# Copy example assets to build directory
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
