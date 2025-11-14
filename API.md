# stevensSound API Reference

Complete API documentation for stevensSound library.

## Table of Contents

- [Initialization](#initialization)
- [Sound Playback](#sound-playback)
- [Memory Management](#memory-management)
- [Playlist Management](#playlist-management)
- [Audio Effects](#audio-effects)
- [Error Handling](#error-handling)
- [RAII Wrappers](#raii-wrappers)
- [Data Structures](#data-structures)

---

## Initialization

### `initSound()`

```cpp
bool initSound()
```

Initializes SDL and SDL_mixer libraries.

**Returns:** `true` if successful, `false` otherwise

**Example:**
```cpp
if (!initSound())
{
    std::cerr << "Failed to initialize\n";
    return 1;
}
```

---

### `closeSound()`

```cpp
void closeSound()
```

Closes SDL and SDL_mixer, frees all resources. Call this before program exit.

**Example:**
```cpp
closeSound();  // Clean shutdown
```

---

### `stevensSound::init()`

```cpp
bool stevensSound::init(
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> soundsParam
)
```

Initializes the stevensSound library with sound definitions.

**Parameters:**
- `soundsParam`: Nested map of `{category -> {name -> filepath}}`

**Returns:** `true` if successful

**Example:**
```cpp
std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
    {"sfx", {
        {"button", "sounds/button.wav"},
        {"error", "sounds/error.wav"}
    }},
    {"music", {
        {"theme", "music/theme.mp3"}
    }}
};

stevensSound::init(sounds);
```

---

## Sound Playback

### `stevensSound::playSound()`

```cpp
void stevensSound::playSound(
    const std::string& category,
    const std::string& soundName,
    const std::string& whenChannelsBusy = "return"
)
```

Plays a sound effect.

**Parameters:**
- `category`: Sound category (e.g., "sfx", "music")
- `soundName`: Name of the sound
- `whenChannelsBusy`: Behavior when all channels are busy:
  - `"return"`: Skip playing (default)
  - `"wait"`: Wait for available channel
  - `"steal"`: Steal a channel (not yet implemented)

**Example:**
```cpp
stevensSound::playSound("sfx", "button_click");
stevensSound::playSound("sfx", "explosion", "wait");  // Wait for channel
```

---

### `stevensSound::playSound_detached()`

```cpp
void stevensSound::playSound_detached(
    const std::string& category,
    const std::string& soundName,
    const std::string& whenChannelsBusy = "return"
)
```

Plays a sound on a detached thread (non-blocking).

**Example:**
```cpp
stevensSound::playSound_detached("sfx", "long_sound");
// Continues immediately without waiting
```

---

### `stevensSound::playPersistentSound()`

```cpp
void stevensSound::playPersistentSound(
    const std::string& category,
    const std::string& soundName,
    const std::string& whenChannelsBusy = "return"
)
```

Plays a persistently loaded sound (faster, no disk I/O).

**Example:**
```cpp
stevensSound::playPersistentSound("sfx", "gunshot");
```

---

## Memory Management

### `stevensSound::storePersistentSound()`

```cpp
void stevensSound::storePersistentSound(
    const std::string& category,
    const std::string& soundName
)
```

Loads a sound into memory for faster playback.

**Example:**
```cpp
// Pre-load frequently used sounds
stevensSound::storePersistentSound("sfx", "button_click");
stevensSound::storePersistentSound("sfx", "hover_sound");
```

---

### `stevensSound::freePersistentSound()`

```cpp
void stevensSound::freePersistentSound(
    const std::string& category,
    const std::string& soundName
)
```

Frees a persistently stored sound from memory.

**Example:**
```cpp
stevensSound::freePersistentSound("sfx", "button_click");
```

---

### `stevensSound::freeChunks()`

```cpp
void stevensSound::freeChunks()
```

Frees all non-persistent chunks that aren't currently playing.

---

### `stevensSound::freePersistentChunks()`

```cpp
void stevensSound::freePersistentChunks()
```

Frees all persistent chunks. Called automatically by `closeSound()`.

---

## Playlist Management

### `stevensSound::createPlaylist()`

```cpp
void stevensSound::createPlaylist(
    std::string playlistName,
    std::string controllerId,
    std::vector<std::string> soundCategoriesUsed,
    std::vector<std::string> trackOrder,
    bool shuffleFill
)
```

Creates a music playlist.

**Parameters:**
- `playlistName`: Unique identifier for the playlist
- `controllerId`: Volume controller to use (e.g., "music")
- `soundCategoriesUsed`: Categories to pull tracks from
- `trackOrder`: Ordered list of track names
- `shuffleFill`: Fill remaining tracks in random order

**Example:**
```cpp
std::vector<std::string> categories = {"music"};
std::vector<std::string> tracks = {"intro", "main_theme", "battle"};

stevensSound::createPlaylist(
    "game_music",      // Name
    "music",           // Controller
    categories,        // Categories
    tracks,            // Track order
    false              // Don't shuffle fill
);
```

---

### `stevensSound::playMusicPlaylist()`

```cpp
void stevensSound::playMusicPlaylist(
    s_soundPlaylist& playlist,
    std::string onCompletion = "end"
)
```

Plays a music playlist (blocking call, use on separate thread).

**Parameters:**
- `playlist`: Reference to playlist object
- `onCompletion`: Behavior when playlist ends:
  - `"end"`: Stop playing (default)
  - `"loop"`: Restart from beginning
  - `"shuffle"`: Shuffle and restart

**Example:**
```cpp
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["game_music"]),
    "loop"
);
musicThread.detach();
```

---

### `stevensSound::switchMusicPlaylist()`

```cpp
void stevensSound::switchMusicPlaylist(const std::string& switchToPlaylist)
```

Switches to a different playlist (thread-safe).

**Example:**
```cpp
stevensSound::switchMusicPlaylist("battle_music");
```

---

### `stevensSound::stopMusicPlaylist()`

```cpp
void stevensSound::stopMusicPlaylist()
```

Stops the currently playing playlist.

**Example:**
```cpp
stevensSound::stopMusicPlaylist();
```

---

## Audio Effects

### `stevensSound::setupAntiFatigueSound()`

```cpp
void stevensSound::setupAntiFatigueSound(
    const std::string& category,
    const std::string& soundName,
    float randomRange = 0.1f
)
```

Sets up pitch/volume randomization to prevent audio fatigue.

**Parameters:**
- `category`: Sound category
- `soundName`: Sound name
- `randomRange`: Variation amount (0.0 to 1.0, default 0.1 = 10%)

**Example:**
```cpp
// Vary button clicks by 10%
stevensSound::setupAntiFatigueSound("sfx", "button_click", 0.1f);

// Vary notifications by 15%
stevensSound::setupAntiFatigueSound("sfx", "notification", 0.15f);
```

---

### `AudioEffectsManager::setEffects()`

```cpp
static void AudioEffectsManager::setEffects(
    const std::string& category,
    const std::string& soundName,
    const AudioEffects& effects
)
```

Sets custom audio effects for a sound.

**Example:**
```cpp
stevensSound::AudioEffects effects;
effects.volumeModulation = 0.8f;   // 80% volume
effects.panPosition = -0.5f;        // Pan left
effects.randomizePitch = true;
effects.randomRange = 0.1f;

stevensSound::AudioEffectsManager::setEffects("sfx", "enemy_hit", effects);
```

---

### `AudioEffectsManager::setPanPosition()`

```cpp
static void AudioEffectsManager::setPanPosition(
    const std::string& category,
    const std::string& soundName,
    float pan
)
```

Sets stereo pan position.

**Parameters:**
- `pan`: -1.0 (full left) to 1.0 (full right), 0.0 = center

**Example:**
```cpp
stevensSound::AudioEffectsManager::setPanPosition("sfx", "car_left", -0.8f);
stevensSound::AudioEffectsManager::setPanPosition("sfx", "car_right", 0.8f);
```

---

## Error Handling

### `ErrorHandler::setErrorHandler()`

```cpp
static void ErrorHandler::setErrorHandler(
    std::function<void(const ErrorInfo&)> handler
)
```

Sets a custom error handler callback.

**Example:**
```cpp
stevensSound::ErrorHandler::setErrorHandler([](const stevensSound::ErrorInfo& error) {
    std::cerr << "[stevensSound] " << error.toString() << "\n";
});
```

---

### `ErrorHandler::setLogging()`

```cpp
static void ErrorHandler::setLogging(bool enable)
```

Enables or disables automatic error logging to stdout.

**Example:**
```cpp
stevensSound::ErrorHandler::setLogging(true);  // Enable logging
```

---

### `ErrorHandler::getLastError()`

```cpp
static ErrorInfo ErrorHandler::getLastError()
```

Gets the last error that occurred on this thread.

**Returns:** `ErrorInfo` structure with error details

**Example:**
```cpp
auto error = stevensSound::ErrorHandler::getLastError();
std::cout << "Error level: " << (int)error.level << "\n";
std::cout << "Message: " << error.message << "\n";
std::cout << "Function: " << error.function << "\n";
```

---

### `ErrorHandler::hasError()`

```cpp
static bool ErrorHandler::hasError()
```

Checks if there is an error set on this thread.

**Example:**
```cpp
if (stevensSound::ErrorHandler::hasError())
{
    std::cout << stevensSound::ErrorHandler::getLastErrorMessage() << "\n";
}
```

---

### `ErrorHandler::clearError()`

```cpp
static void ErrorHandler::clearError()
```

Clears the current error state.

---

## RAII Wrappers

### `MixChunkRAII`

RAII wrapper for `Mix_Chunk*` that automatically frees resources.

```cpp
class MixChunkRAII
{
    explicit MixChunkRAII(const char* path);
    Mix_Chunk* get() const;
    bool isValid() const;
    Mix_Chunk* release();
};
```

**Example:**
```cpp
{
    stevensSound::MixChunkRAII chunk("sound.wav");
    if (chunk.isValid())
    {
        Mix_PlayChannel(-1, chunk.get(), 0);
    }
    // Automatically freed here
}
```

---

### `MixMusicRAII`

RAII wrapper for `Mix_Music*`.

```cpp
class MixMusicRAII
{
    explicit MixMusicRAII(const char* path);
    Mix_Music* get() const;
    bool isValid() const;
    Mix_Music* release();
};
```

---

### Factory Functions

```cpp
MixChunkPtr makeMixChunk(const char* path);
MixMusicPtr makeMixMusic(const char* path);
```

**Example:**
```cpp
auto chunk = stevensSound::makeMixChunk("sound.wav");
auto music = stevensSound::makeMixMusic("theme.mp3");
```

---

## Data Structures

### `ErrorInfo`

```cpp
struct ErrorInfo
{
    ErrorLevel level;        // INFO, WARNING, ERROR, CRITICAL
    std::string message;     // Error message
    std::string function;    // Function where error occurred
    std::string timestamp;   // When the error occurred

    std::string toString() const;
};
```

---

### `AudioEffects`

```cpp
struct AudioEffects
{
    float pitchVariation;     // -1.0 to 1.0 (reserved for future use)
    float volumeModulation;   // 0.0 to 1.0
    float panPosition;        // -1.0 (left) to 1.0 (right)
    bool randomizePitch;      // Enable pitch randomization
    float randomRange;        // Range of variation (0.0 to 1.0)
};
```

---

### `s_soundController`

```cpp
class s_soundController
{
public:
    std::string id;      // Controller identifier
    float volume;        // Volume (0.0 to 1.0)
};
```

**Access:**
```cpp
stevensSound::soundControllers["sfx"].volume = 0.8f;
stevensSound::soundControllers["music"].volume = 0.5f;
```

---

## Global Variables

### Sound Controllers

```cpp
stevensSound::soundControllers["default"]  // Default controller
stevensSound::soundControllers["sfx"]      // Sound effects
stevensSound::soundControllers["music"]    // Music
```

### Playlists

```cpp
stevensSound::playlists["playlist_name"]  // Access playlists
```

### Sounds

```cpp
stevensSound::sounds[category][name]  // Access sound data
```

---

## Thread Safety Notes

- **Thread-safe:** Error handling (uses thread_local)
- **Thread-safe:** Sound playback (uses mutexes)
- **Thread-safe:** Playlist switching
- **Not thread-safe:** Direct modification of `soundControllers` map (use carefully)

---

## Best Practices

1. **Always check errors** in critical paths
2. **Use persistent loading** for frequently played sounds
3. **Use anti-fatigue** for repetitive UI sounds
4. **Play playlists on separate threads**
5. **Clean up with `closeSound()`** before exit
6. **Use RAII wrappers** for custom sound management

---

For more examples, see the `examples/` directory.
