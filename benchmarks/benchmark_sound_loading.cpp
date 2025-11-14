/**
 * @file benchmark_sound_loading.cpp
 * @brief Benchmarks for sound loading operations
 */

#include <benchmark/benchmark.h>
#include "../stevensSound.hpp"

// Benchmark initialization
static void BM_InitSound(benchmark::State& state)
{
    for (auto _ : state)
    {
        state.PauseTiming();
        closeSound();
        state.ResumeTiming();

        initSound();

        state.PauseTiming();
        closeSound();
        state.ResumeTiming();
    }
}
BENCHMARK(BM_InitSound);

// Benchmark library initialization
static void BM_InitLibrary(benchmark::State& state)
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
BENCHMARK(BM_InitLibrary);

// Benchmark error handling overhead
static void BM_ErrorHandling(benchmark::State& state)
{
    using namespace stevensSound;

    for (auto _ : state)
    {
        ErrorHandler::setError(ErrorLevel::ERROR, "Benchmark error", "BenchmarkFunction");
        benchmark::DoNotOptimize(ErrorHandler::getLastError());
        ErrorHandler::clearError();
    }
}
BENCHMARK(BM_ErrorHandling);

// Benchmark soundsContains check
static void BM_SoundsContains(benchmark::State& state)
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
BENCHMARK(BM_SoundsContains);
