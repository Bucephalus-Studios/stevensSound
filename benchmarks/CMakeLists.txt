# Benchmarks for stevensSound library

# Create benchmark executable
add_executable(stevensSound_benchmarks
    benchmark_main.cpp
    benchmark_sound_loading.cpp
    benchmark_sound_playback.cpp
)

target_link_libraries(stevensSound_benchmarks
    PRIVATE
        stevensSound
        benchmark::benchmark
        benchmark::benchmark_main
)

# Copy benchmark assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_assets
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
