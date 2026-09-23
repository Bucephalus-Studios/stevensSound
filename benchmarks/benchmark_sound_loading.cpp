/**
 * @file benchmark_sound_loading.cpp
 * @brief Benchmarks for sound loading operations
 */

#include <benchmark/benchmark.h>
#include "../stevensSound.hpp"

// Benchmark initialization
static void InitSound(benchmark::State& state)
{
    for (auto _ : state)
    {
        initSound();

        state.PauseTiming();
        closeSound();
        state.ResumeTiming();
    }
}
BENCHMARK(InitSound);

// Benchmark library initialization
static void InitLibrary(benchmark::State& state)
{
    initSound();

    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}},
        {"music", {}}
    };

    for (auto _ : state)
    {
        stevensSound::init(sounds);
    }

    closeSound();
}
BENCHMARK(InitLibrary);

// Benchmark soundsContains check
static void SoundsContains(benchmark::State& state)
{
    initSound();

    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}},
        {"music", {}}
    };

    stevensSound::init(sounds);

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(stevensSound::soundsContains("sfx", "test"));
    }

    closeSound();
}
BENCHMARK(SoundsContains);
