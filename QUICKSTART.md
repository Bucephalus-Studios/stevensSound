# stevensSound Quick Start Guide

Get up and running with stevensSound in 5 minutes!

## Step 1: Integration (Recommended)

The easiest way to use stevensSound is via CMake's `add_subdirectory`:

### In your CMakeLists.txt:

```cmake
# Add stevensSound to your project
add_subdirectory(path/to/stevensSound)

# Link it to your executable
target_link_libraries(your_app PRIVATE stevensSound)
```

**That's it!** CMake automatically downloads and links:
- SDL2 (v2.28.5)
- SDL2_mixer (v2.6.3)
- stevensVectorLib (for sound-variant selection)

No manual dependency installation required!

## Step 2: Standalone Build (Optional)

To build tests, examples, and benchmarks:

```bash
git clone <repository-url>
cd stevensSound
mkdir build && cd build
cmake ..
cmake --build .
```

Tests, examples, and benchmarks are automatically built when stevensSound is the main project (i.e. not pulled in via `add_subdirectory` from another project).

## Step 3: Your First Sound

```cpp
#include <stevensSound.hpp>

int main()
{
    // 1. Initialize SDL
    initSound();

    // 2. Define your sounds
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {
            {"button", "assets/button.wav"}
        }}
    };

    // 3. Initialize library
    stevensSound::init(sounds);

    // 4. Play a sound!
    stevensSound::playSound("sfx", "button");

    // 5. Clean up
    closeSound();
    return 0;
}
```

## Common Use Cases

### Sound Variants for Anti-Fatigue

Register several files under the same sound name — `playSound()` picks one at random each time, avoiding the "same click every time" fatigue:

```cpp
stevensSound::sounds["sfx"]["select"] = stevensSound::createSound(
    "select", "sfx", "sfx",
    "assets/sfx/select1.wav",
    { "assets/sfx/select2.wav", "assets/sfx/select3.wav" }  // variants
);

// Later: plays a random variant during gameplay
stevensSound::playSound("sfx", "select");
```

### Background Music

```cpp
// Create a playlist and register it
std::vector<std::string> categories = {"music"};
std::vector<std::string> tracks = {"song1", "song2", "song3"};

stevensSound::playlists["bgm"] = stevensSound::createMusicPlaylist(
    "bgm", "music", categories, tracks, false
);

// Play on separate thread
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["bgm"]),
    "loop"  // Loop forever
);
musicThread.detach();

// Later: adjust volume (applied to the running music thread)
stevensSound::setMusicVolume(0.3f);
```

### Volume Control

```cpp
// Individual controllers for different sound types
stevensSound::soundControllers["sfx"].volume = 0.8f;     // 80% sfx volume
stevensSound::soundControllers["music"].volume = 0.5f;   // 50% music volume
```

### Error Handling

```cpp
// Enable error logging
stevensSound::ErrorHandler::setLogging(true);

// Try to play a sound
stevensSound::playSound("sfx", "nonexistent");

// Check for errors
if (stevensSound::ErrorHandler::hasError())
{
    std::cout << "Error: "
              << stevensSound::ErrorHandler::getLastErrorMessage()
              << "\n";
}
```

## Running the Examples

```bash
cd build
./examples/basic
./examples/error_handling
./examples/playlist
```

## Testing

```bash
# Run unit tests
./tests/tests

# Run performance benchmarks
./benchmarks/benchmarks
```

## Next Steps

- Read the full [README.md](README.md) for an overview
- Check out the examples in `examples/` directory
- Explore the full API reference in [API.md](API.md)

## Troubleshooting

### "Failed to initialize SDL"

Make sure audio drivers are available on your system. On Linux, you may need:

```bash
sudo apt-get install libasound2-dev libpulse-dev
```

### CMake can't find SDL2

The library automatically downloads SDL2. If you have issues:

```bash
# Clean build directory
rm -rf build/*
cmake ..
```

### Sounds don't play

1. Check file paths are correct (relative to executable)
2. Verify audio format is supported (WAV is safest)
3. Enable error logging to see what's happening:

```cpp
stevensSound::ErrorHandler::setLogging(true);
```

## Platform Notes

### Linux
- Fully supported
- May need audio development packages

### Windows
- Fully supported
- SDL2 will be built with Windows audio backend

### macOS
- Fully supported
- Uses Core Audio backend

## Getting Help

- Check the examples directory
- Review test cases in `tests/`
- Read the full API documentation
- File an issue on GitHub

Happy coding! 🎵