# stevensSound

An easy-to-use, modern C++ audio library built on top of SDL2 and SDL2_mixer, featuring automatic resource management, comprehensive error handling, and audio effects.

## Features

- **Zero-dependency installation**: SDL2 and SDL2_mixer are automatically downloaded and statically linked
- **Modern C++20**: RAII wrappers, smart pointers, and move semantics
- **Comprehensive error handling**: Thread-safe error system with custom handlers
- **Audio effects**: Pitch modulation, panning, and anti-fatigue randomization
- **Playlist management**: Create, manage, and control music playlists
- **Thread-safe**: Safe for multi-threaded applications
- **Header-only**: Simple integration into your project
- **Fully tested**: Includes Google Test suite and Google Benchmark

## Quick Start

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Basic Usage

```cpp
#include <stevensSound.hpp>

int main()
{
    // Initialize SDL
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL\n";
        return 1;
    }

    // Set up your sounds
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {
            {"button_click", "sounds/button.wav"},
            {"notification", "sounds/notify.wav"}
        }},
        {"music", {
            {"theme", "music/theme.mp3"}
        }}
    };

    // Initialize the library
    stevensSound::init(sounds);

    // Play a sound
    stevensSound::playSound("sfx", "button_click");

    // Control volume
    stevensSound::soundControllers["sfx"].volume = 0.5f;  // 50% volume

    // Clean up
    closeSound();
    return 0;
}
```

## Key Features

### 1. Error Handling

```cpp
// Enable logging
stevensSound::ErrorHandler::setLogging(true);

// Set custom error handler
stevensSound::ErrorHandler::setErrorHandler([](const stevensSound::ErrorInfo& error) {
    std::cout << "Error: " << error.message << "\n";
});

// Check for errors
if (stevensSound::ErrorHandler::hasError())
{
    auto error = stevensSound::ErrorHandler::getLastError();
    std::cout << error.toString() << "\n";
}
```

### 2. RAII Resource Management

```cpp
// Automatic cleanup with RAII wrappers
{
    auto chunk = stevensSound::makeMixChunk("sound.wav");
    if (chunk && chunk->isValid())
    {
        // Use the sound
        Mix_PlayChannel(-1, chunk->get(), 0);
    }
    // Automatically freed when out of scope
}
```

### 3. Audio Effects & Anti-Fatigue

```cpp
// Set up anti-fatigue for repetitive UI sounds
stevensSound::setupAntiFatigueSound("sfx", "button_click", 0.1f);

// The sound will now have slight pitch/volume variation on each play
stevensSound::playSound("sfx", "button_click");  // Slightly different each time

// Custom effects
stevensSound::AudioEffects effects;
effects.volumeModulation = 0.8f;   // 80% volume
effects.panPosition = -0.5f;        // Pan to left
effects.randomizePitch = true;
effects.randomRange = 0.15f;        // 15% variation

stevensSound::AudioEffectsManager::setEffects("sfx", "notification", effects);
```

### 4. Playlist Management

```cpp
// Create a music playlist
std::vector<std::string> categories = {"music"};
std::vector<std::string> trackOrder = {"song1", "song2", "song3"};

stevensSound::createPlaylist(
    "my_playlist",
    "music",
    categories,
    trackOrder,
    false  // Don't shuffle fill
);

// Play the playlist on a separate thread
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["my_playlist"]),
    "loop"  // Loop when finished
);
musicThread.detach();

// Switch to another playlist
stevensSound::switchMusicPlaylist("another_playlist");

// Stop the music
stevensSound::stopMusicPlaylist();
```

### 5. Persistent Sound Loading

```cpp
// Pre-load frequently used sounds into memory
stevensSound::storePersistentSound("sfx", "button_click");

// Now this sound plays with minimal latency (no disk I/O)
stevensSound::playSound("sfx", "button_click");  // Fast!

// Free when no longer needed
stevensSound::freePersistentSound("sfx", "button_click");
```

## SDL Versions

This library is designed to work with:
- **SDL2**: Version 2.0.22
- **SDL2_mixer**: Version 2.6.3+

**Note:** Due to CMake/Makefile issues with SDL2_mixer's FetchContent integration, we recommend installing SDL2/SDL2_mixer via your system package manager for the best experience. See [BUILD_NOTES.md](BUILD_NOTES.md) for details and workarounds.

## Building Options

```bash
# Build with all features (default)
cmake ..

# Build without tests
cmake -DSTEVENSSOUND_BUILD_TESTS=OFF ..

# Build without benchmarks
cmake -DSTEVENSSOUND_BUILD_BENCHMARKS=OFF ..

# Build without examples
cmake -DSTEVENSSOUND_BUILD_EXAMPLES=OFF ..
```

## Running Tests

```bash
# Build and run tests
cmake --build . --target stevensSound_tests
./bin/stevensSound_tests

# Run benchmarks
cmake --build . --target stevensSound_benchmarks
./bin/stevensSound_benchmarks
```

## Examples

The `examples/` directory contains complete working examples:

- `basic_example.cpp` - Basic library usage
- `error_handling_example.cpp` - Error handling features
- `playlist_example.cpp` - Playlist management
- `raii_example.cpp` - RAII wrappers
- `pitch_modulation_example.cpp` - Audio effects

Build and run:

```bash
cmake --build . --target basic_example
./bin/basic_example
```

## Project Structure

```
stevensSound/
├── stevensSound.hpp           # Main library header
├── classes/                   # Library components
│   ├── s_soundData.h         # Sound data structure
│   ├── s_soundController.h   # Volume controller
│   ├── s_soundPlaylist.h     # Playlist management
│   ├── s_errorHandler.h      # Error handling system
│   ├── s_raii_wrappers.h     # RAII resource wrappers
│   └── s_pitchModulation.h   # Audio effects
├── libraries/                 # Utility libraries
│   ├── stevensSetLib.h       # Set utilities
│   └── stevensFileLib.hpp    # File utilities
├── tests/                     # Google Test suite
├── benchmarks/                # Google Benchmark suite
├── examples/                  # Usage examples
└── CMakeLists.txt            # Build configuration
```

## API Reference

### Initialization

```cpp
bool initSound();                          // Initialize SDL/SDL_mixer
void closeSound();                         // Clean up SDL/SDL_mixer
bool stevensSound::init(sounds);           // Initialize library with sounds
```

### Sound Playback

```cpp
void stevensSound::playSound(category, name, whenChannelsBusy);
void stevensSound::playSound_detached(category, name, whenChannelsBusy);
void stevensSound::playPersistentSound(category, name, whenChannelsBusy);
```

### Memory Management

```cpp
void stevensSound::storePersistentSound(category, name);
void stevensSound::freePersistentSound(category, name);
void stevensSound::freeChunks();
void stevensSound::freePersistentChunks();
```

### Playlist Management

```cpp
void stevensSound::createPlaylist(name, controllerId, categories, trackOrder, shuffleFill);
void stevensSound::playMusicPlaylist(playlist, onCompletion);
void stevensSound::switchMusicPlaylist(playlistName);
void stevensSound::stopMusicPlaylist();
```

### Audio Effects

```cpp
// Anti-fatigue setup
void stevensSound::setupAntiFatigueSound(category, name, randomRange);

// Effects management
void stevensSound::AudioEffectsManager::setEffects(category, name, effects);
AudioEffects stevensSound::AudioEffectsManager::getEffects(category, name);
void stevensSound::AudioEffectsManager::setPitchVariation(category, name, variation);
void stevensSound::AudioEffectsManager::setPanPosition(category, name, pan);
```

### Error Handling

```cpp
void stevensSound::ErrorHandler::setErrorHandler(handler);
void stevensSound::ErrorHandler::setLogging(enable);
ErrorInfo stevensSound::ErrorHandler::getLastError();
std::string stevensSound::ErrorHandler::getLastErrorMessage();
bool stevensSound::ErrorHandler::hasError();
void stevensSound::ErrorHandler::clearError();
```

## Important Notes

### Pitch Shifting Limitation

SDL_mixer does not natively support true pitch shifting without tempo changes. The pitch modulation features in this library provide:
- Volume modulation for perceived pitch variation
- Panning effects
- Random variation to prevent audio fatigue

For true pitch shifting, consider integrating:
- [SoLoud](https://solhsa.com/soloud/)
- [libsoundtouch](https://www.surina.net/soundtouch/)
- PortAudio with custom DSP

### Thread Safety

- Error handling is thread-safe (uses thread_local storage)
- Sound playback uses mutexes for resource pool management
- Playlist switching is synchronized with audio thread

### Supported Audio Formats

Thanks to SDL_mixer, supports:
- WAV, AIFF, VOC (native)
- MP3, OGG, FLAC (when compiled with support)
- MOD, XM, IT (tracker formats)

## Performance

The library includes benchmarks for performance testing:

```bash
./bin/stevensSound_benchmarks
```

Key optimizations:
- Persistent sound caching for frequently used sounds
- RAII wrappers prevent memory leaks
- Efficient resource pooling
- Lock-free error handling per thread

## Contributing

When contributing, please:
1. Run tests: `./bin/stevensSound_tests`
2. Run benchmarks to check for regressions
3. Follow existing code style
4. Add tests for new features

## License

See LICENSE file for details.

## Credits

- Built on [SDL2](https://www.libsdl.org/) and [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)
- Uses [Google Test](https://github.com/google/googletest) for testing
- Uses [Google Benchmark](https://github.com/google/benchmark) for performance testing

## Version History

### 1.0.0 (2025-11-14)
- Initial release
- Comprehensive error handling system
- RAII resource wrappers
- Audio effects and anti-fatigue features
- CMake build system with automatic SDL dependency management
- Full test and benchmark suite
- Complete documentation and examples
