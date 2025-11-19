/**
 * @file benchmark_sound_playback.cpp
 * @brief Benchmarks for sound playback operations
 */

#include <benchmark/benchmark.h>
#include "../stevensSound.hpp"

// Benchmark playlist creation
static void CreatePlaylist(benchmark::State& state)
{
    initSound();

    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"music", {}}
    };

    stevensSound::init(sounds);

    std::vector<std::string> categories = {"music"};
    std::vector<std::string> trackOrder = {};

    int counter = 0;
    for (auto _ : state)
    {
        std::string playlistName = "test_playlist_" + std::to_string(counter++);
        stevensSound::createPlaylist(playlistName, "music", categories, trackOrder, false);
    }

    closeSound();
}
BENCHMARK(CreatePlaylist);

// Benchmark volume controller access
static void VolumeControllerAccess(benchmark::State& state)
{
    initSound();

    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}}
    };

    stevensSound::init(sounds);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(stevensSound::soundControllers["sfx"].volume);
    }

    closeSound();
}
BENCHMARK(VolumeControllerAccess);

// Benchmark isPersistentlyStored check
static void IsPersistentlyStored(benchmark::State& state)
{
    initSound();

    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}}
    };

    stevensSound::init(sounds);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(stevensSound::isPersistentlyStored("sfx", "test"));
    }

    closeSound();
}
BENCHMARK(IsPersistentlyStored);

// Benchmark error checking
static void ErrorChecking(benchmark::State& state)
{
    using namespace stevensSound;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(ErrorHandler::hasError());
    }
}
BENCHMARK(ErrorChecking);
