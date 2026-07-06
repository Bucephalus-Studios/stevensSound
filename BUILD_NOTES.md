# Build Notes

## Known Issues

### SDL2_mixer FetchContent Build Issue

There is a known issue with SDL2_mixer 2.6.3 and 2.8.1 when building via FetchContent in certain environments. The error manifests as:

```
target pattern contains no '%'.  Stop.
```

This is a CMake/Makefile generation issue in SDL2_mixer's build system, not an issue with stevensSound itself. It is specific to the `Unix Makefiles` generator.

### Workaround Options

#### Option 0: Use the Ninja Generator (Recommended)

Switching generators avoids the issue entirely, since it's specific to Makefile generation:

```bash
cmake .. -G Ninja
ninja
```

#### Option 1: Use System SDL2 (Recommended for Development)

Install SDL2 and SDL2_mixer on your system:

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev libsdl2-mixer-dev
```

**macOS:**
```bash
brew install sdl2 sdl2_mixer
```

**Windows:**
Download from https://www.libsdl.org/

Then disable FetchContent in CMakeLists.txt and use find_package instead.

#### Option 2: Manual SDL2_mixer Build

1. Clone and build SDL2 and SDL2_mixer manually
2. Install them to a prefix directory
3. Point CMake to that installation:

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/sdl/install
```

#### Option 3: Compile Without the CMake Build System

`stevensSound.cpp` is a regular translation unit — you can compile it directly instead of using the provided CMakeLists.txt:

```bash
g++ -std=c++20 -c stevensSound.cpp -I. -I/path/to/stevensVectorLib -I/path/to/stevensMathLib $(pkg-config --cflags sdl2 SDL2_mixer)
g++ -std=c++20 my_app.cpp stevensSound.o -lSDL2 -lSDL2_mixer -lpthread
```

## Library Code Status

- ✅ Error handling system
- ✅ Playlist management
- ✅ Sound variants (anti-fatigue random selection)
- ✅ Thread-safe resource management
- ✅ Test suite
- ✅ Benchmark suite
- ✅ Examples

Pitch modulation and RAII wrapper classes were prototyped in earlier revisions but removed — they depended on a since-dropped SoundTouch dependency / were never wired into the public header. See git history if you want to revisit either.

The Makefile-generation issue above is purely with SDL2_mixer's own build scripts, not stevensSound itself.

## Recommended Usage

For production use, we recommend:
1. Build via the root CMakeLists.txt with the Ninja generator (see Option 0 above), or
2. Install SDL2/SDL2_mixer via your system's package manager and use `find_package` instead of FetchContent

Both avoid the Makefile-generation issue entirely.

## Future Work

- Consider switching to vcpkg or Conan for dependency management
- Investigate using prebuilt SDL2 binaries
- Add support for multiple dependency management systems
