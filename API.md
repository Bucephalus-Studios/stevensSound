# stevensSound API Reference

Complete API documentation for stevensSound library.

## Table of Contents

- [Initialization](#initialization)
- [Sound & Music Creation](#sound--music-creation)
- [Sound Playback](#sound-playback)
- [Memory Management](#memory-management)
- [Playlist Management](#playlist-management)
- [Error Handling](#error-handling)
- [Data Structures](#data-structures)
- [Global Variables](#global-variables)

---

## Initialization

### `initSound()`

```cpp
bool initSound()
```

Initializes SDL and SDL_mixer libraries. Call once at program startup.

**Returns:** `true` if successful, `false` otherwise

---

### `closeSound()`

```cpp
void closeSound()
```

Halts all playback, frees all cached sounds/music, and shuts down SDL_mixer/SDL. Call this before program exit.

---

### `stevensSound::init()`

```cpp
bool stevensSound::init(
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> soundsParam
)
```

Creates the `"music"`, `"sfx"`, and `"default"` playback controllers, then loads every sound/music file described by `soundsParam`.

**Parameters:**
- `soundsParam`: Nested map of `{category -> {name -> filepath}}`. Any category whose name **contains the substring `"music"`** is loaded as streamed `Mix_Music` (into `stevensSound::music`); every other category is loaded as a `Mix_Chunk` (into `stevensSound::sounds`).

**Returns:** `true` (load failures are reported via `stderr`/`ErrorHandler`, not the return value)

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

## Sound & Music Creation

### `stevensSound::createSound()`

```cpp
Sound stevensSound::createSound(
    std::string name,
    std::string type,
    std::string controllerId,
    const char* filePath,
    std::vector<const char*> variantFilePaths = {}
)
```

Loads a `Sound` (and any variant files) into memory and returns it. Used internally by `stevensSound::init()`, but can also be called directly to add a sound after startup (store the result into `stevensSound::sounds[category][name]`).

### `stevensSound::createMusic()`

```cpp
Music stevensSound::createMusic(
    std::string name,
    std::string type,
    std::string controllerId,
    const char* filePath
)
```

Loads a `Music` handle into memory and returns it.

### `stevensSound::loadSoundData()` / `stevensSound::loadMusicData()`

```cpp
std::unordered_map<std::string, Sound> stevensSound::loadSoundData(
    std::unordered_map<std::string, const char*> soundNamesnPaths,
    std::string soundType,
    std::string controllerId
);

std::unordered_map<std::string, Music> stevensSound::loadMusicData(
    std::unordered_map<std::string, const char*> musicNamesnPaths,
    std::string musicType,
    std::string controllerId
);
```

Batch versions of `createSound()`/`createMusic()`, used internally by `init()`.

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

Plays a sound effect on a free SDL_mixer channel. If the sound has variants (see `createSound`), one is chosen at random each call.

**Parameters:**
- `whenChannelsBusy`: Behavior when all 16 channels are busy:
  - `"return"` (default): Skip playing
  - `"wait"`: Busy-wait (10ms poll) for a free channel
  - `"steal"`: Not yet implemented — currently a no-op, so the sound will not play

**Example:**
```cpp
stevensSound::playSound("sfx", "button_click");
stevensSound::playSound("sfx", "explosion", "wait");
```

### `stevensSound::playSound_detached()`

```cpp
void stevensSound::playSound_detached(
    const std::string& category,
    const std::string& soundName,
    const std::string& whenChannelsBusy = "return"
)
```

Runs `playSound()` on a detached `std::thread` so the caller doesn't block (relevant for `whenChannelsBusy = "wait"`).

### `stevensSound::soundsContains()`

```cpp
bool stevensSound::soundsContains(
    const std::string& category,
    const std::string& soundName
)
```

Returns `true` if a loaded sound effect exists under `category`/`soundName`. (Only checks `stevensSound::sounds`, not `stevensSound::music`.)

---

## Memory Management

### `stevensSound::freeChunks()`

```cpp
void stevensSound::freeChunks()
```

Frees any chunks tracked in the internal chunk pool that have finished playing. Safe to call periodically; chunks still playing are left alone.

### `stevensSound::freeMixChunks()`

```cpp
void stevensSound::freeMixChunks()
```

Frees every `Mix_Chunk` cached in `stevensSound::sounds`. Called automatically by `closeSound()` during shutdown — don't call this while sounds might still be needed.

---

## Playlist Management

### `stevensSound::createMusicPlaylist()` / `stevensSound::createSoundPlaylist()`

```cpp
SoundPlaylist stevensSound::createMusicPlaylist(
    std::string playlistName,
    std::string controllerId,
    std::vector<std::string> musicCategoriesUsed,
    std::vector<std::string> trackOrder,
    bool shuffleFill
);

SoundPlaylist stevensSound::createSoundPlaylist(
    std::string playlistName,
    std::string controllerId,
    std::vector<std::string> soundCategoriesUsed,
    std::vector<std::string> soundOrder,
    bool shuffleFill
);
```

Builds a `SoundPlaylist` from the requested categories/track order and returns it — it is **not** automatically registered. Store it into `stevensSound::playlists[playlistName]` yourself so `switchMusicPlaylist()` can find it later.

**Parameters:**
- `playlistName`: Identifier for the playlist (also stored on the returned object)
- `controllerId`: Which `soundControllers` entry controls this playlist's volume (e.g. `"music"`)
- `musicCategoriesUsed`/`soundCategoriesUsed`: Categories to pull tracks from
- `trackOrder`/`soundOrder`: Explicit ordering; any name not found logs a warning to `stderr` and is skipped
- `shuffleFill`: If `true`, appends all remaining tracks from the given categories in random order after `trackOrder`

**Example:**
```cpp
std::vector<std::string> categories = {"music"};
std::vector<std::string> tracks = {"intro", "main_theme", "battle"};

stevensSound::playlists["game_music"] = stevensSound::createMusicPlaylist(
    "game_music", "music", categories, tracks, false
);
```

### `stevensSound::playMusicPlaylist()`

```cpp
void stevensSound::playMusicPlaylist(
    SoundPlaylist& playlist,
    std::string onCompletion = "end"
)
```

Blocking call — run it on its own `std::thread`. Plays through `playlist`'s tracks in order, polling an internal command queue (fed by `switchMusicPlaylist()`, `stopMusicPlaylist()`, `setMusicVolume()`) roughly every 100ms while a track plays.

**Parameters:**
- `onCompletion`: `"end"` (default, stop), `"loop"` (restart from track 0), or `"shuffle"` (shuffle and restart)

**Example:**
```cpp
std::thread musicThread(
    stevensSound::playMusicPlaylist,
    std::ref(stevensSound::playlists["game_music"]),
    "loop"
);
musicThread.detach();
```

### `stevensSound::playPlaylist()`

```cpp
void stevensSound::playPlaylist(SoundPlaylist playlist)
```

Synchronously plays every sound in a playlist in order (via `playSound()`), honoring `preTrackDelays`/`postTrackDelays`. Typically used for sound-effect sequences rather than music.

### `stevensSound::switchMusicPlaylist()`

```cpp
void stevensSound::switchMusicPlaylist(
    const std::string& switchToPlaylist,
    PlaylistSwitchOptions options = {}
)
```

Fire-and-forget: queues a switch command for whichever thread is running `playMusicPlaylist()`. Errors (unknown playlist name) are reported via `ErrorHandler`, not a return value.

```cpp
stevensSound::switchMusicPlaylist("battle_music", { .fadeInMs = 500 });
```

### `stevensSound::stopMusicPlaylist()`

```cpp
void stevensSound::stopMusicPlaylist()
```

Queues a stop command and **blocks** until the music thread acknowledges it.

### `stevensSound::setMusicVolume()` / `stevensSound::setSfxVolume()`

```cpp
void stevensSound::setMusicVolume(float volume);
void stevensSound::setSfxVolume(float volume);
```

Update the `"music"`/`"sfx"` controller's volume (0.0–1.0). `setMusicVolume()` also queues a command so the running music thread applies it immediately; `setSfxVolume()` takes effect on each subsequent `playSound()` call.

---

## Error Handling

### `ErrorHandler::setErrorHandler()`

```cpp
static void ErrorHandler::setErrorHandler(std::function<void(const ErrorInfo&)> handler)
```

Sets a custom callback invoked every time the library reports an error.

### `ErrorHandler::setLogging()`

```cpp
static void ErrorHandler::setLogging(bool enable)
```

When enabled, every reported error is also printed to `stdout`.

### `ErrorHandler::getLastError()` / `getLastErrorMessage()`

```cpp
static ErrorInfo ErrorHandler::getLastError();
static std::string ErrorHandler::getLastErrorMessage();
```

Returns the last error set **on the calling thread** (error state is `thread_local`).

### `ErrorHandler::hasError()`

```cpp
static bool ErrorHandler::hasError()
```

### `ErrorHandler::clearError()`

```cpp
static void ErrorHandler::clearError()
```

**Example:**
```cpp
stevensSound::ErrorHandler::setLogging(true);
stevensSound::ErrorHandler::setErrorHandler([](const stevensSound::ErrorInfo& error) {
    std::cerr << "[stevensSound] " << error.toString() << "\n";
});
```

---

## Data Structures

### `ErrorInfo`

```cpp
struct ErrorInfo
{
    ErrorLevel level;        // INFO, WARNING, ERROR, CRITICAL
    std::string message;
    std::string function;
    std::string timestamp;

    std::string toString() const;
};
```

### `Sound`

```cpp
class Sound
{
public:
    std::string name, type, controllerId;
    Mix_ChunkData mainChunkData;
    std::vector<Mix_ChunkData> variantChunkData;

    bool hasVariants() const;
    bool isValid() const;
    Mix_Chunk* getChunkToPlay();  // random variant if any exist
    void freeAllChunks();
};
```

### `Music`

```cpp
class Music
{
public:
    std::string name, type, controllerId;
    Mix_MusicData musicData;

    bool isValid() const;
    void freeMusic();
};
```

### `PlaybackController`

```cpp
class PlaybackController
{
public:
    std::string id;
    float volume;  // 0.0 to 1.0
};
```

**Access:**
```cpp
stevensSound::soundControllers["sfx"].volume = 0.8f;
stevensSound::soundControllers["music"].volume = 0.5f;
```

### `SoundPlaylist`

```cpp
class SoundPlaylist
{
public:
    std::string name;
    std::vector<std::tuple<std::string,std::string>> sounds;  // (category, name) in order
    int index;
    std::string controllerId;
    std::string status;  // "stopped", "paused", or "playing"
    std::unordered_map<int,int> preTrackDelays;   // ms delay before track at this index
    std::unordered_map<int,int> postTrackDelays;  // ms delay after track at this index
    double trackPosition = 0.0;  // seconds into the current track; saved/restored across switches

    void shuffle();
    std::string getName() const;
};
```

### `PlaylistSwitchOptions`

```cpp
struct PlaylistSwitchOptions
{
    int fadeInMs = 0;  // if > 0, fades in the incoming playlist's first track
};
```

### `Mix_ChunkData` / `Mix_MusicData`

Global (non-namespaced) wrappers pairing a file path with its loaded SDL handle:

```cpp
class Mix_ChunkData
{
public:
    std::string filePath;
    Mix_Chunk* chunk;
    bool load();
    void free();
    bool isLoaded() const;
};

class Mix_MusicData
{
public:
    std::string filePath;
    Mix_Music* music;
    bool load();
    void free();
    bool isLoaded() const;
};
```

---

## Global Variables

```cpp
stevensSound::soundControllers  // unordered_map<string, PlaybackController> — "default", "sfx", "music"
stevensSound::playlists         // unordered_map<string, SoundPlaylist> — starts with a "currently playing" entry
stevensSound::sounds            // unordered_map<category, unordered_map<name, Sound>>
stevensSound::music             // unordered_map<category, unordered_map<name, Music>>
```

---

## Thread Safety Notes

- **Thread-safe:** Error handling (`thread_local` storage)
- **Thread-safe:** Chunk pool bookkeeping (mutex-guarded)
- **Coordinated via command queue, not free-threaded:** `switchMusicPlaylist()`/`stopMusicPlaylist()`/`setMusicVolume()` are safe to call from any thread, but only make sense while exactly one thread is running `playMusicPlaylist()`
- **Not thread-safe:** Direct modification of `soundControllers`/`sounds`/`music`/`playlists` maps (only mutate them from one thread at a time, typically before spinning up the music thread)

## Supported Audio Formats

Thanks to SDL_mixer, and as configured in this library's `CMakeLists.txt`:
- WAV, AIFF, VOC (native)
- MP3, OGG (enabled)
- FLAC, MOD/XM/IT (disabled by default — flip the corresponding `SDL2MIXER_*` option in `CMakeLists.txt` to enable)

For more examples, see the `examples/` directory.