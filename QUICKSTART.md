# stevensSound Quick Start Guide

Get up and running with stevensSound in 5 minutes!

## Step 1: Build the Library

```bash
git clone <repository-url>
cd stevensSound
mkdir build && cd build
cmake ..
cmake --build .
```

That's it! CMake automatically downloads and builds SDL2 and SDL2_mixer for you.

## Step 2: Basic Integration

### Option A: CMake Integration

Add to your `CMakeLists.txt`:

```cmake
add_subdirectory(stevensSound)
target_link_libraries(your_app PRIVATE stevensSound)
```

### Option B: Header-Only

Just include the main header:

```cpp
#include "stevensSound.hpp"
```

Make sure to link SDL2 and SDL2_mixer.

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

### UI Sounds with Anti-Fatigue

Prevent annoying repetition in UI sounds:

```cpp
// Set up once
stevensSound::setupAntiFatigueSound("sfx", "button_click", 0.1f);

// Play multiple times - each will sound slightly different
for (int i = 0; i < 10; i++)
{
    stevensSound::playSound("sfx", "button_click");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}
```

### Background Music

```cpp
// Create a playlist
std::vector<std::string> categories = {"music"};
std::vector<std::string> tracks = {"song1", "song2", "song3"};

stevensSound::createPlaylist("bgm", "music", categories, tracks, false);

// Play on separate thread
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["bgm"]),
    "loop"  // Loop forever
);
musicThread.detach();

// Later: adjust volume
stevensSound::soundControllers["music"].volume = 0.3f;
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

### Pre-loading Sounds

For sounds that need instant playback (e.g., gunshots in a game):

```cpp
// Load into memory once
stevensSound::storePersistentSound("sfx", "gunshot");

// Now plays instantly (no disk I/O)
stevensSound::playSound("sfx", "gunshot");  // Fast!

// Free when done
stevensSound::freePersistentSound("sfx", "gunshot");
```

## Running the Examples

```bash
cd build
./bin/basic_example
./bin/error_handling_example
./bin/pitch_modulation_example
./bin/raii_example
./bin/playlist_example
```

## Testing

```bash
# Run unit tests
./bin/stevensSound_tests

# Run performance benchmarks
./bin/stevensSound_benchmarks
```

## Next Steps

- Read the full [README.md](README.md) for detailed API documentation
- Check out the examples in `examples/` directory
- Explore the API reference in [API.md](API.md)

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
