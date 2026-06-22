#pragma once

/**
 * An easy-to-use C++ library to interface with SDL 2 for playing sounds in applications.
*/
//Standard libraries used
#include<iostream>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<tuple>
#include<algorithm>
#include<random>
#include<cmath>
#include<cstdio>
#include<atomic>
#include<condition_variable>
#include<memory>
#include<mutex>
#include<queue>
#include<thread>

#if defined(__linux__)
    #include<SDL2/SDL.h>
    #include<SDL2/SDL_mixer.h>
#elif defined(_WIN32)
    #include<SDL2/SDL.h>
	#include<SDL2/SDL_mixer.h>
#endif

//Include global (non-namespaced) SDL wrapper classes
#include "classes/Mix_ChunkData.h"
#include "classes/Mix_MusicData.h"

//Include stevensSound classes (each wraps itself in namespace stevensSound)
#include "classes/AudioCommand.hpp"
#include "classes/ErrorHandler.hpp"
#include "classes/Music.hpp"
#include "classes/PlaybackController.hpp"
#include "classes/PlaylistSwitchOptions.hpp"
#include "classes/Sound.hpp"
#include "classes/SoundPlaylist.hpp"
// NOTE: Pitch modulation feature is untested and still a stub implementation
// #include "classes/s_pitchModulation.h"

/***** Global variables and helpers for managing Mix_Chunk memory (defined in stevensSound.cpp) *****/
static std::mutex stevensSound_chunkMutex;
static std::unordered_set< Mix_Chunk* > stevensSound_chunkPool;

bool stevensSound_isChunkPlaying( Mix_Chunk * chunk );
void stevensSound_channelFinishedCallback( const int channel );
/**************************************************************************/

/**
 * STEVENSOUND RESOURCE OWNERSHIP AND LIFECYCLE MODEL
 *
 * MEMORY MANAGEMENT:
 * - All SDL resources (Mix_Music*, Mix_Chunk*) are loaded ONCE at startup via loadAllMusic()/loadAllSounds()
 * - Resources are stored in global caches ('music' and 'sounds' maps) and persist for the entire program lifetime
 * - Playback functions (playMusicPlaylist, playSound) REFERENCE cached resources but DO NOT own them
 * - Resources are ONLY freed during program shutdown in cleanUp()
 *
 * THREADING MODEL:
 * - Main thread: Loads resources, switches playlists, handles user input
 * - Music thread: Runs playMusicPlaylist() in background, plays music continuously
 * - Sound threads: Detached threads for sound effect playback
 * - Synchronization: audioChange flag with busy-wait (TODO: upgrade to condition variables)
 *
 * CRITICAL RULES:
 * - NEVER call Mix_FreeMusic() or Mix_FreeChunk() outside of cleanUp()
 * - NEVER modify the 'music' or 'sounds' maps after loading (read-only after init)
 * - ALWAYS validate resources with isValidMusic()/isValidSound() before playback
 * - When switching playlists, use Mix_HaltMusic() to stop playback (NOT Mix_FreeMusic)
 */
namespace stevensSound
{
	/*** Variables ***/
	extern std::unordered_map<std::string, std::unordered_map<std::string, Sound> > sounds;
	extern std::unordered_map<std::string, std::unordered_map<std::string, Music> > music;
	extern std::unordered_map<std::string, SoundPlaylist> playlists;
	extern std::unordered_map<std::string, PlaybackController> soundControllers;

	/*** Methods ***/
	std::string getSDLVersionInfo();

	Sound createSound(	std::string name,
						std::string type,
						std::string controllerId,
						const char* filePath,
						std::vector<const char*> variantFilePaths = {}	);

	Music createMusic(	std::string name,
						std::string type,
						std::string controllerId,
						const char* filePath	);

	std::unordered_map<std::string, Sound> loadSoundData(	std::unordered_map<std::string, const char *> soundNamesnPaths,
															std::string soundType,
															std::string controllerId  );

	std::unordered_map<std::string, Music> loadMusicData(	std::unordered_map<std::string, const char *> musicNamesnPaths,
															std::string musicType,
															std::string controllerId  );

	bool init(	std::unordered_map<std::string, std::unordered_map<std::string, const char *> > soundsParam    );

	void freeChunks();
	void freeMixChunks();

	bool soundsContains(	const std::string & category,
							const std::string & soundName	);

	void playSound(	const std::string & category,
					const std::string & soundName,
					const std::string & whenChannelsBusy = "return"	);

	void playSound_detached(	const std::string & category,
								const std::string & soundName,
								const std::string & whenChannelsBusy = "return"	);

	SoundPlaylist createMusicPlaylist(	std::string playlistName,
										std::string controllerId,
										std::vector<std::string> musicCategoriesUsed,
										std::vector<std::string> trackOrder,
										bool shuffleFill	);

	SoundPlaylist createSoundPlaylist(	std::string playlistName,
										std::string controllerId,
										std::vector<std::string> soundCategoriesUsed,
										std::vector<std::string> soundOrder,
										bool shuffleFill	);

	void switchMusicPlaylist(	const std::string & switchToPlaylist,
								PlaylistSwitchOptions options = {}	);

	void stopMusicPlaylist();

	void setMusicVolume( float volume );
	void setSfxVolume(   float volume );

	void playMusicPlaylist(	SoundPlaylist & playlist,
							std::string onCompletion = "end");

	void playPlaylist(	SoundPlaylist playlist	);
}

/**
 * @brief Initializes the SDL and SDL mixer libraries so they can be interfaced with.
 * @return True if initialization was successful, false otherwise
*/
bool initSound();

/**
 * @brief Closes the SDL and SDL mixer libraries.
*/
void closeSound();
