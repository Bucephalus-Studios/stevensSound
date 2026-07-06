# stevensSound

An easy-to-use, modern C++ audio library built on top of SDL2 and SDL2_mixer, featuring automatic resource management, comprehensive error handling, and playlist management.

## Features

- **Zero-dependency installation**: SDL2, SDL2_mixer, and stevensVectorLib are automatically fetched and statically linked via CMake FetchContent
- **Modern C++20**
- **Comprehensive error handling**: Thread-safe error system with custom handlers
- **Sound variants**: Register multiple audio files under one sound name to get random variation on each play (useful for avoiding repetitive UI/SFX fatigue)
- **Playlist management**: Create, switch between, and control music/sound playlists from a background thread
- **Thread-safe playback**: Music playback runs on its own thread and is driven by a small command queue (switch/stop/volume)
- **Fully tested**: Includes a Google Test suite and Google Benchmark suite

## Quick Start

### Building

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Ninja works too, and avoids a known Makefile-generation issue in SDL2_mixer's build (see [BUILD_NOTES.md](BUILD_NOTES.md)):

```bash
mkdir build && cd build
cmake .. -G Ninja
ninja
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

    // Set up your sounds. Any category whose name contains "music" is loaded
    // as streamed Mix_Music; every other category is loaded as a Mix_Chunk.
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

### 2. Sound Variants (Anti-Fatigue)

Register several files under the same sound; each call to `playSound()` picks one at random.

```cpp
stevensSound::Sound footstep = stevensSound::createSound(
    "footstep", "sfx", "sfx",
    "sounds/footstep1.wav",
    { "sounds/footstep2.wav", "sounds/footstep3.wav" }  // variants
);
stevensSound::sounds["sfx"]["footstep"] = footstep;

stevensSound::playSound("sfx", "footstep");  // plays a random variant each time
```

### 3. Playlist Management

```cpp
// Create a music playlist and register it
std::vector<std::string> categories = {"music"};
std::vector<std::string> trackOrder = {"song1", "song2", "song3"};

stevensSound::playlists["game_music"] = stevensSound::createMusicPlaylist(
    "game_music",
    "music",
    categories,
    trackOrder,
    false  // Don't shuffle fill
);

// Play the playlist on a separate thread
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["game_music"]),
    "loop"  // Loop when finished
);
musicThread.detach();

// Switch to another playlist (fire-and-forget; picked up by the music thread)
stevensSound::switchMusicPlaylist("another_playlist");

// Stop the music (blocks until the music thread acknowledges)
stevensSound::stopMusicPlaylist();
```

## SDL Versions

This library fetches and builds against:
- **SDL2**: Version 2.28.5
- **SDL2_mixer**: Version 2.6.3

**Note:** Due to CMake/Makefile issues with SDL2_mixer's FetchContent integration in some environments, you can alternatively install SDL2/SDL2_mixer via your system package manager. See [BUILD_NOTES.md](BUILD_NOTES.md) for details and workarounds.

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

These options only default to `ON` when stevensSound is built as the top-level project. If you add it via `add_subdirectory()` from your own game's CMake project, none of tests/examples/benchmarks are built unless you opt in.

## Running Tests

```bash
# Build and run tests
cmake --build . --target tests
./tests/tests

# Build and run benchmarks
cmake --build . --target benchmarks
./benchmarks/benchmarks
```

## Examples

The `examples/` directory contains complete working examples:

- `basic_example.cpp` - Basic library usage
- `error_handling_example.cpp` - Error handling features
- `playlist_example.cpp` - Playlist management

Build and run:

```bash
cmake --build . --target basic
./examples/basic
```

## Project Structure

```
stevensSound/
├── stevensSound.hpp           # Main library header (includes classes/ below)
├── stevensSound.cpp            # Compiled implementation
├── classes/                    # Library components
│   ├── AudioCommand.hpp       # Internal command-queue message type
│   ├── ErrorHandler.hpp       # Thread-safe error handling system
│   ├── Mix_ChunkData.h        # Wraps a Mix_Chunk + its file path
│   ├── Mix_MusicData.h        # Wraps a Mix_Music + its file path
│   ├── Music.hpp               # A named, loaded piece of music
│   ├── PlaybackController.hpp # Per-category volume controller
│   ├── PlaylistSwitchOptions.hpp
│   ├── Sound.hpp               # A named sound with optional variants
│   └── SoundPlaylist.hpp       # Ordered list of sound/music keys
├── tests/                      # Google Test suite
├── benchmarks/                 # Google Benchmark suite
├── examples/                   # Usage examples
└── CMakeLists.txt              # Build configuration
```

## API Reference

See [API.md](API.md) for the full API reference.

## Important Notes

### No True Pitch Shifting

SDL_mixer does not natively support true pitch shifting without tempo changes, and this library does not implement it. A libsoundtouch-based prototype was explored but removed as incomplete. If you need pitch shifting, integrate a DSP library such as [libsoundtouch](https://www.surina.net/soundtouch/) or [SoLoud](https://solhsa.com/soloud/) directly in your application, or use `createSound()`'s variant files for perceived variation instead.

### Resource Lifecycle

- All sounds/music passed to `stevensSound::init()` are loaded once and cached for the lifetime of the program.
- Never call `Mix_FreeChunk()`/`Mix_FreeMusic()` on cached resources yourself — `closeSound()` frees everything on shutdown.
- Extra chunks played via `playSound()` (not part of the initial load) are tracked in an internal pool and freed automatically when they finish playing, or via `stevensSound::freeChunks()`.

### Thread Safety

- Error handling is thread-safe (uses thread_local storage)
- `playMusicPlaylist()` is meant to run on a dedicated thread; switching/stopping/volume changes from other threads are delivered via an internal command queue
- Sound playback uses mutexes for resource pool management

### Supported Audio Formats

Thanks to SDL_mixer, supports:
- WAV, AIFF, VOC (native)
- MP3, OGG (enabled by this library's CMake configuration)
- FLAC, tracker formats (MOD/XM/IT) are disabled by default in CMakeLists.txt but can be re-enabled there

## Contributing

When contributing, please:
1. Run tests: `./tests/tests`
2. Run benchmarks to check for regressions
3. Follow existing code style
4. Add tests for new features

## License

See LICENSE file for details.

## Credits

- Built on [SDL2](https://www.libsdl.org/) and [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer)
- Uses [stevensVectorLib](https://github.com/Bucephalus-Studios/stevensVectorLib) for variant selection
- Uses [Google Test](https://github.com/google/googletest) for testing
- Uses [Google Benchmark](https://github.com/google/benchmark) for performance testing
