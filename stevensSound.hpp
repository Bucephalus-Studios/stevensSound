#ifndef STEVENS_SOUND_HPP
#define STEVENS_SOUND_HPP

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
#include<memory>
#include<mutex>
#include<thread>

/*
SDL supports windows and mac, so we may not need platform specific sound for windows mac and linux. Possibly for mobile though.
*/
#if defined(__linux__)
    #include<SDL2/SDL.h>
    #include<SDL2/SDL_mixer.h>
#elif defined(_WIN32)
    #include<SDL2/SDL.h>
	#include<SDL2/SDL_mixer.h>
#endif

//Custom libraries used here
// #include "libraries/stevensSetLib.h"

//Include common classes that are used globally
#include "classes/Mix_ChunkData.h"
#include "classes/Mix_MusicData.h"


/***** Global functions and variables used for managing Mix_Chunk memory *****/
static std::mutex stevensSound_chunkMutex;
static std::unordered_set< Mix_Chunk* > stevensSound_chunkPool;


/**
 * @brief Checks to see if a given Mix_Chunk is actively playing on a channel
 */
inline bool stevensSound_isChunkPlaying( Mix_Chunk * chunk )
{
	// Iterate all channels (Mix_AllocateChannels(-1) returns the current number of channels)
	for (int i = 0; i < Mix_AllocateChannels(-1); i++)
	{
        if (Mix_GetChunk(i) == chunk && Mix_Playing(i))
		{
            return true;  // Chunk is actively playing on this channel
        }
    }
    return false;
}


/**
 * @brief When a channel finishes playing, frees the Mix_Chunk from memory.
 */
inline void stevensSound_channelFinishedCallback( const int channel )
{
	//stevensFileLib::appendToFile( "stevensSound.log", "Channel finished: " + std::to_string(channel) + "\n" );

	//Get the chunk from the channel
	Mix_Chunk * chunk = Mix_GetChunk(channel);
	//If there happens to be no chunk, return. (Shouldn't happen tho)
	if( !chunk )
	{
		//stevensFileLib::appendToFile( "stevensSound.log", "No chunk found on channel: " + std::to_string(channel) + "\n" );
		return;
	}

	//Lock other threads for modification of the chunk pool
	std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);
    if( stevensSound_chunkPool.contains(chunk) )
	{
		// Remove from pool if present
		stevensSound_chunkPool.erase(chunk);
        Mix_FreeChunk(chunk);
		//stevensFileLib::appendToFile( "stevensSound.log", "Chunk freed from " + std::to_string(channel) + "\n" );
    }

	//stevensFileLib::appendToFile( "stevensSound.log", "Callback finished for " + std::to_string(channel) + "\n" );
}
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
	//Include classes for the library (inside namespace)
	// NOTE: ErrorHandler must be included FIRST because Sound and Music use it in isValid()
	#include "classes/ErrorHandler.hpp"
	#include "classes/Sound.hpp"
	#include "classes/Music.hpp"
	#include "classes/PlaybackController.hpp"
	#include "classes/SoundPlaylist.hpp"
	// NOTE: Pitch modulation feature is untested and still a stub implementation
	// It requires the SoundTouch library which is not currently installed
	// #include "classes/s_pitchModulation.h"

	/*** Variables ***/
	inline std::unordered_map<std::string, std::unordered_map<std::string, Sound> > sounds; //Container of all sounds (Mix_Chunks stored in Sound objects) - OWNED by stevensSound, freed in cleanUp()
	inline std::unordered_map<std::string, std::unordered_map<std::string, Music> > music; //Container of all music (Mix_Music stored in Music objects) - OWNED by stevensSound, freed in cleanUp()
	inline std::unordered_map<std::string, SoundPlaylist> playlists; //Contains all of the playlists created with the stevensSound library
	inline std::unordered_map<std::string, PlaybackController> soundControllers; //Container of all playback controllers. Controls volume and playback settings for sounds.
	inline bool audioChange; //Bool used to control when currently audio playback is modified in any way


	/*** Methods ***/
	/**
	 * @brief Returns a std::string containing the version information of SDL and SDL_mixer.
	 */
	inline 	std::string getSDLVersionInfo()
	{
		SDL_version sdl_ver, mix_ver;
    	SDL_GetVersion(&sdl_ver);
    	MIX_VERSION(&mix_ver);

		std::string sdlVersionString = std::to_string((int)sdl_ver.major) + "." + std::to_string((int)sdl_ver.minor) + "." + std::to_string((int)sdl_ver.patch);
		std::string sdlMixerString = std::to_string((int)mix_ver.major) + "." + std::to_string((int)mix_ver.minor) + "." + std::to_string((int)mix_ver.patch);

		std::string versionInfo = "SDL Version: " + sdlVersionString + "\n" +
								  "SDL Mixer Version: " + sdlMixerString + "\n";
		return versionInfo;
	}


	/**
	 * @brief Factory function to create a Sound object and load its Mix_Chunks into memory
	 *
	 * @param name The name of the sound
	 * @param type The type/category of the sound (e.g., "sfx", "music")
	 * @param controllerId The ID of the sound controller
	 * @param filePath Path to the main sound file
	 * @param variantFilePaths Optional vector of paths to pitch-shifted variants
	 * @return Sound object with loaded Mix_Chunks
	 */
	inline Sound createSound(	std::string name,
								std::string type,
								std::string controllerId,
								const char* filePath,
								std::vector<const char*> variantFilePaths = {}	)
	{
		Sound data(name, type, controllerId);

		//Load main chunk
		data.mainChunkData = Mix_ChunkData(filePath);
		if( !data.mainChunkData.load() )
		{
			std::cerr << "CRITICAL: Failed to load main sound chunk for '" << name << "'" << std::endl;
		}

		//Load variant chunks if provided (skip any that fail to load)
		for(const char* variantPath : variantFilePaths)
		{
			Mix_ChunkData variantData(variantPath);
			if( variantData.load() )
			{
				data.variantChunkData.push_back(variantData);
			}
			//Note: load() already prints error message on failure
		}

		return data;
	}


	/**
	 * @brief Factory function to create a Music object and load its Mix_Music handle into memory
	 *
	 * @param name The name of the music track
	 * @param type The type/category of the music (e.g., "music", "battle music")
	 * @param controllerId The ID of the sound controller
	 * @param filePath Path to the music file
	 * @return Music object with loaded Mix_Music
	 */
	inline Music createMusic(	std::string name,
								std::string type,
								std::string controllerId,
								const char* filePath	)
	{
		Music data(name, type, controllerId);

		//Load music handle
		data.musicData = Mix_MusicData(filePath);
		if( !data.musicData.load() )
		{
			std::cerr << "CRITICAL: Failed to load music for '" << name << "'" << std::endl;
		}

		return data;
	}


	/**
	 * @brief Given an unordered_map with key-value pairs of sound names and file paths to the sounds respectively and 
	 * a type of sound, return an unordered_map with key value pairs of sound names and Sound objects
	 * containing data about the sounds. 
	 * 
	 * Parameters:
	 *  unordered_map<std::string, const char *> soundNamesnPaths - A map of key-value pairs of sound names and their
	 *                                                              file paths relative to the executable file location
	 *                                                              respectively.
	 *  std::string soundType - The type of sound to assign to all of the newly constructed Sound objects.
	 * 	std::string controllerId - Id of the controller to which to assign each of the newly loaded sounds to.
	 * 
	 * Returns:
	 *  unordered_map<std::string, Sound> - A map of sound objects where the key-value pairs are of the sound name
	 *                                            and the data object containing all necessary information about the 
	 *                                            sound needed for it to be used in this library respectively.
	 *  
	*/
	inline 	std::unordered_map<std::string, Sound> loadSoundData(	std::unordered_map<std::string, const char *> soundNamesnPaths,
																std::string soundType,
																std::string controllerId  )
	{
		std::unordered_map<std::string, Sound> soundDataMap = {};
		Sound soundData;

		//Iterate through all of the sound names and paths
		for(auto & [soundName,path] : soundNamesnPaths)
		{
			//Use factory function to create and load the sound
			soundDataMap[soundName] = createSound(soundName, soundType, controllerId, path);
		}

		return soundDataMap;
	}


	/**
	 * @brief Given an unordered_map with key-value pairs of music names and file paths to the music respectively and
	 * a type of music, return an unordered_map with key value pairs of music names and Music objects
	 * containing data about the music.
	 *
	 * Parameters:
	 *  unordered_map<std::string, const char *> musicNamesnPaths - A map of key-value pairs of music names and their
	 *                                                              file paths relative to the executable file location
	 *                                                              respectively.
	 *  std::string musicType - The type of music to assign to all of the newly constructed Music objects.
	 * 	std::string controllerId - Id of the controller to which to assign each of the newly loaded music tracks to.
	 *
	 * Returns:
	 *  unordered_map<std::string, Music> - A map of music objects where the key-value pairs are of the music name
	 *                                            and the data object containing all necessary information about the
	 *                                            music needed for it to be used in this library respectively.
	 *
	*/
	inline 	std::unordered_map<std::string, Music> loadMusicData(	std::unordered_map<std::string, const char *> musicNamesnPaths,
															std::string musicType,
															std::string controllerId  )
	{
		std::unordered_map<std::string, Music> musicDataMap = {};

		//Iterate through all of the music names and paths
		for(auto & [musicName,path] : musicNamesnPaths)
		{
			//Use factory function to create and load the music
			musicDataMap[musicName] = createMusic(musicName, musicType, controllerId, path);
		}

		return musicDataMap;
	}


	/**
	 * @brief Initializes the sound controllers and loads sound data into this library's sounds map.createPlaylist
	 * 		  Must be called before using this library.
	 *
	 * @param soundsParam An outer map of sound categories as keys paired with values of maps with keys of
	 * 					  sound names and values of filepaths to the sounds.
	 * 						{	{	"music",	{	{"song1", "ogeechee hymnal.wav"},
	 * 												{"song2", "the sky is red.wav"},
	 * 												{"song3", "the hampster dance.wav"}	}	},
	 * 							{	"sfx",		{	{"flapjackScream",	"oooahahhah.wav"},
	 * 												{"YTMND",	"youreTheManNowDog.wav"	}	}	}
	 * @return True if initialization was successful, false otherwise
	 */
	inline 	bool init(	std::unordered_map<std::string, std::unordered_map<std::string, const char *> > soundsParam    )
	{
		//Create music, sfx, and default playback controllers
		soundControllers = {	{"music",	PlaybackController("music",1)},
								{"sfx",		PlaybackController("sfx",1)},
								{"default",	PlaybackController()}		};

		//Initialize sounds and music separately
		sounds = {};
		music = {};
		for(auto & [soundType, soundMap] : soundsParam)
		{
			//Check if this is music or SFX
			if( soundType.find("music") != std::string::npos )
			{
				//Load as music (uses Mix_Music)
				music[soundType] = loadMusicData(soundMap, soundType, soundType);
			}
			else
			{
				//Load as sound effect (uses Mix_Chunk)
				sounds[soundType] = loadSoundData(soundMap, soundType, soundType);
			}
		}

		audioChange = false;

		int channels = Mix_AllocateChannels(16);
		if (channels != 16)
		{
			ErrorHandler::setError(ErrorLevel::WARNING,
				"Failed to allocate 16 channels, got " + std::to_string(channels),
				"init");
		}

		//Initialize our playlists container to be empty other than having an empty "currently playing" playlist
		playlists.emplace( "currently playing", SoundPlaylist() );

		ErrorHandler::setError(ErrorLevel::INFO, "stevensSound initialized successfully", "init");
		return true;
	}


	/**
	 * @brief Frees all of the chunks that we've allocated in non-persistent storage
	 */
	inline 	void freeChunks()
	{
		std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);
		std::vector<Mix_Chunk*> toDelete;

		// Separate still-playing chunks from deletable ones
		for (auto* chunk : stevensSound_chunkPool)
		{
			if ( !stevensSound_isChunkPlaying(chunk) )
			{
				toDelete.push_back(chunk);
			}
		}

		// Free only unused chunks
		for (auto* chunk : toDelete)
		{
			Mix_FreeChunk(chunk);
			stevensSound_chunkPool.erase(chunk);
		}
	}


	/**
	 * @brief Used to free all of the chunks stored in memory during shutdown of stevensSound library and SDL Mixer.
	 */
	inline 	void freeMixChunks()
	{
		std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);

		//Free all chunks stored in sound data objects
		for( auto & [category, soundMap] : sounds )
		{
			for( auto & [soundName, soundData] : soundMap )
			{
				soundData.freeAllChunks();
			}
		}
	}


	/**
	 * @brief Returns true if we have an entry in the sounds map with a matching category and
	 * 		  soundName.
	 */
	inline 	bool soundsContains(	const std::string & category,
							const std::string & soundName	)
	{
		if( sounds.contains( category ) )
		{
			return sounds.at( category ).contains( soundName );
		}
		return false;
	}


	/**
	 * Given the name of a sound and the category under which it is stored, play the sound with SDL Mixer on a free audio channel.
	 * 
	 * Parameters:
	 * 	std::string soundName - The string name of a sound under a category in the stevensSound object's sound map that you wish to play.
	 * 	std::string category - The string category name of a sound within the stevenSound object's sound map that you wish to play.
	 * 	@param stealChannel If true, if all mix channels are taken by currently playing sounds, steal the channel from a playing sound
	 * 						to play this sound. If false, wait until a channel is open to play this sound.
	 * 
	 * Returns:
	 * 	void, but plays sounds
	*/
	inline 	void playSound(	const std::string & category,
					const std::string & soundName,
					const std::string & whenChannelsBusy = "return"	)
	{
		//Check: Is the category and soundName combo valid?
		if( !soundsContains( category, soundName ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Requested to play sound with category \"" + category + "\" and name \"" + soundName + "\", but it does not exist",
				"playSound");
			return;
		}

		//Get a random chunk to play (handles anti-fatigue variant selection)
		Mix_Chunk* sound = sounds[category][soundName].getChunkToPlay();

		//Check if the chunk is actually loaded
		if( sound == nullptr )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Sound \"" + category + "/" + soundName + "\" exists but is not loaded in memory",
				"playSound");
			return;
		}

		//Control the playback
		Mix_VolumeChunk(sound, (int)std::round(128 * soundControllers[sounds[category][soundName].controllerId].volume));

		/***** PLAY THE SOUND *****/
		int channel = -1;
		if( whenChannelsBusy == "return" )
		{
			//Try to find an open channel
			channel = Mix_PlayChannel( -1, sound, 0 );
		}
		else if( whenChannelsBusy == "wait" )
		{
			//Wait until we have an available channel
			while ( channel == -1 )
			{
				channel = Mix_PlayChannel( -1, sound, 0 );
				if (channel == -1)
				{
					SDL_Delay(10); // Small delay to prevent busy-waiting
				}
			}
		}
		else //steal channel
		{
			//TODO FIND CHANNEL TO STEAL
			void(0);
		}
	}


	inline 	void playSound_detached(	const std::string & category,
								const std::string & soundName,
								const std::string & whenChannelsBusy = "return"	)
	{
		std::thread soundThread = std::thread(	playSound,
												category,
												soundName,
												whenChannelsBusy );
		soundThread.detach();
	}


	/**
	 * Creates an s_soundPlaylist object with the given parameters and stores it in the playlists map under a given name.
	 * 
	 * Parameters:
	 * 	std::string playlistName - The name the newly created playlist will be stored under as a key in the playlists map
	 * 	std::string controllerId - The id of the sound controller that will control the volume for the playlist
	 * 	std::vector<std::string> soundCategoriesUsed - The sound category keys within the sounds map which will be used in the playlist.
	 * 	std::vector<std::string> trackOrder - The names of the tracks in order that will be used in the playlist.
	 * 	bool shuffleFill - If true, fill the rest of the playlist with the rest of the unused sounds from the sound categories being used in a random order.
	 * 
	 * Returns:
	 * 	void, but creates a playlist that will be stored under the given playlist name in the playlists map.
	*/
	inline SoundPlaylist createMusicPlaylist(	std::string playlistName,
												std::string controllerId,
												std::vector<std::string> musicCategoriesUsed,
												std::vector<std::string> trackOrder,
												bool shuffleFill	)
	{
		SoundPlaylist playlist;
		playlist.name = playlistName;
		playlist.controllerId = controllerId;
		playlist.index = 0;
		playlist.status = "stopped";

		//Create a temporary map for tracks we'll add
		std::unordered_map<std::string, std::unordered_map<std::string, Music>> tracksToAdd = {};
		for(const auto& category : musicCategoriesUsed)
		{
			tracksToAdd[category] = music[category];
		}

		//Add tracks in the specified order
		bool foundTrack = false;
		for(const auto& trackName : trackOrder)
		{
			for(const auto& category : musicCategoriesUsed)
			{
				if(tracksToAdd[category].contains(trackName))
				{
					playlist.sounds.push_back(std::make_tuple(category, trackName));
					tracksToAdd[category].erase(trackName);
					foundTrack = true;
					break;
				}
			}
			if(!foundTrack)
			{
				std::cerr << "createMusicPlaylist: Unable to find track '" << trackName << "' in music map.\n";
			}
			else
			{
				foundTrack = false;
			}
		}

		//Shuffle-fill remaining tracks if requested
		if(shuffleFill)
		{
			while(!tracksToAdd.empty())
			{
				//Pick random category
				auto category_it = tracksToAdd.begin();
				std::advance(category_it, rand() % tracksToAdd.size());
				std::string random_category = category_it->first;

				//Pick random track from that category
				auto track_it = tracksToAdd[random_category].begin();
				std::advance(track_it, rand() % tracksToAdd[random_category].size());
				std::string random_track = track_it->first;

				//Add to playlist
				playlist.sounds.push_back(std::make_tuple(random_category, random_track));
				tracksToAdd[random_category].erase(random_track);

				//Remove empty categories
				if(tracksToAdd[random_category].empty())
				{
					tracksToAdd.erase(random_category);
				}
			}
		}

		return playlist;
	}


	inline SoundPlaylist createSoundPlaylist(	std::string playlistName,
												std::string controllerId,
												std::vector<std::string> soundCategoriesUsed,
												std::vector<std::string> soundOrder,
												bool shuffleFill	)
	{
		SoundPlaylist playlist;
		playlist.name = playlistName;
		playlist.controllerId = controllerId;
		playlist.index = 0;
		playlist.status = "stopped";

		//Create a temporary map for sounds we'll add
		std::unordered_map<std::string, std::unordered_map<std::string, Sound>> soundsToAdd = {};
		for(const auto& category : soundCategoriesUsed)
		{
			soundsToAdd[category] = sounds[category];
		}

		//Add sounds in the specified order
		bool foundSound = false;
		for(const auto& soundName : soundOrder)
		{
			for(const auto& category : soundCategoriesUsed)
			{
				if(soundsToAdd[category].contains(soundName))
				{
					playlist.sounds.push_back(std::make_tuple(category, soundName));
					soundsToAdd[category].erase(soundName);
					foundSound = true;
					break;
				}
			}
			if(!foundSound)
			{
				std::cerr << "createSoundPlaylist: Unable to find sound '" << soundName << "' in sounds map.\n";
			}
			else
			{
				foundSound = false;
			}
		}

		//Shuffle-fill remaining sounds if requested
		if(shuffleFill)
		{
			while(!soundsToAdd.empty())
			{
				//Pick random category
				auto category_it = soundsToAdd.begin();
				std::advance(category_it, rand() % soundsToAdd.size());
				std::string random_category = category_it->first;

				//Pick random sound from that category
				auto sound_it = soundsToAdd[random_category].begin();
				std::advance(sound_it, rand() % soundsToAdd[random_category].size());
				std::string random_sound = sound_it->first;

				//Add to playlist
				playlist.sounds.push_back(std::make_tuple(random_category, random_sound));
				soundsToAdd[random_category].erase(random_sound);

				//Remove empty categories
				if(soundsToAdd[random_category].empty())
				{
					soundsToAdd.erase(random_category);
				}
			}
		}

		return playlist;
	}


	/**
	 * @brief Used to switch the content of the currently playing playlist in the playMusicPlaylist function
	 * 		  on a separate thread.
	 * 
	 * @param switchToPlaylist The playlist that we are switching to start playing the music of
	 */
	inline 	void switchMusicPlaylist(	const std::string & switchToPlaylist	)
	{
		//Before we try a switch, make sure the playlist we want to switch to exists
		if( !playlists.contains( switchToPlaylist ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Playlist \"" + switchToPlaylist + "\" does not exist",
				"switchMusicPlaylist");
			return;
		}
		//Also make sure the playlist we are switching from still is stored under the same key in playlists
		if( !playlists.contains( playlists.at("currently playing").getName() ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Currently playing playlist \"" + playlists.at("currently playing").getName() + "\" does not exist in playlists map",
				"switchMusicPlaylist");
			return;
		}

		//Set the currently playing playlist to start the switching process. This will trigger an if statement in playMusicPlaylist()
		playlists.at("currently playing").status = "switching";
        audioChange = true;

		//Wait for the music change to be completed in the playMusicPlaylist() function
		while( audioChange )
		{
			SDL_Delay(10);
		}

		//Store the currrent state of the current playlist before switching it
		playlists.at( playlists.at("currently playing").getName()  ) = playlists.at("currently playing");

		//Switch the playlist now
		playlists.at("currently playing") = playlists.at(switchToPlaylist);

		//Start up the music again
		playlists.at("currently playing").status = "playing";
		audioChange = true;
	}


	/**
	 * @brief Stops the currently playing playlist on a separate thread.
	 */
	inline 	void stopMusicPlaylist()
	{
		//Stop the current music
		playlists.at("currently playing").status = "stopped";
		audioChange = true;

		//Wait for the music to stop
		while(audioChange)
		{
			SDL_Delay(100);
		}

		return;
	}


	/**
	 * @brief Initiates playing a playlist for music. Typically used on a separate std::thread than the main one.
	 * 		  Use this function for playing a playlist of sounds that you would want to be able to pause, modify
	 * 		  the volume of, loop, shuffle between sounds, etc.
	 * 
	 * @param playlist
	 * @param onCompletion
	 * 
	 * Returns:
	 * 		void
	 * 
	 **/
	inline 	void playMusicPlaylist(	SoundPlaylist & playlist,
							std::string onCompletion = "end")
	{
		playlist.status = "playing";
		//Label that we jump to when switching playlists
		beginPlaylist:
		while(playlist.status != "playing")
		{
			SDL_Delay(10);
		}
		std::string categoryName; //The category of the sound that is currently playing
		std::string soundName; //The name of the sound that is currently playing
		Mix_Music* currentSound = nullptr;

		while(true)
		{
			//Play music until the playlist finishes
			while(playlist.index < playlist.sounds.size())
			{
				//Get the data from thes sound in the playlist
				categoryName = std::get<0>(playlist.sounds[playlist.index]);
				soundName = std::get<1>(playlist.sounds[playlist.index]);
				//Before we play the track, if we have a preTrackDelay, delay here
				if( playlist.preTrackDelays.contains(playlist.index) )
				{
					SDL_Delay( playlist.preTrackDelays.at(playlist.index) );
				}
				//Get the Music object from the cache
				Music& musicToPlay = music[categoryName][soundName];

				//Validate the music before playing it
				if (!musicToPlay.isValid())
				{
					// Error already logged by Music::isValid()
					SDL_ClearError();  // Clear the error state
					playlist.index++;
					continue;  // Skip to next track
				}

				//Get the Mix_Music handle
				currentSound = musicToPlay.musicData.music;

				//Play the music!
				Mix_PlayMusic(currentSound, 1);

				while( Mix_PlayingMusic() )
				{
					if( audioChange ) //If a change in music settings is detected, apply the change to the current playing music
					{
						/*** Switch the playlist to another ***/
						if (playlist.status == "switching")
						{
							Mix_HaltMusic();
							// DO NOT free the music - it's cached in the global music map for reuse
							// Mix_FreeMusic(currentSound);  // REMOVED - was causing use-after-free
							currentSound = nullptr;
							audioChange = false;
							//Go to the top of this function
							goto beginPlaylist;
						}
						/*** If playlist set to stopped, stop playing this playlist and exit this function ***/
						if	(playlist.status == "stopped")
						{
							Mix_HaltMusic();
							// DO NOT free the music - it's cached in the global music map for reuse
							// Mix_FreeMusic(currentSound);  // REMOVED - was causing use-after-free
							audioChange = false;
							return;
						}
						/*** Apply volume changes ***/
						Mix_VolumeMusic(128 * soundControllers[playlist.controllerId].volume);
						/*** Pausing and unpausing playlist ***/
						if ( soundControllers[playlist.controllerId].volume == 0) //If the volume becomes 0, pause the song
						{
							Mix_PauseMusic();
							playlist.status = "paused";
						}
						else if(playlist.status == "paused")
						{
							Mix_PauseMusic();
						}
						else if(playlist.status == "playing")
						{
							Mix_ResumeMusic();
						}
						else if ( Mix_PausedMusic() && ( playlist.status != "paused" ) ) //If the music volume is not 0 and the music is paused, we resume playing.
						{
							Mix_ResumeMusic();
							playlist.status = "playing";
						}
						audioChange = false;
					}
					//We hang out here in this loop while the music is playing
					SDL_Delay(100);
				}
				//When the track finishes playing, if we have a postTrackDelay, delay here
				if( playlist.postTrackDelays.contains(playlist.index) )
				{
					SDL_Delay( playlist.postTrackDelays.at(playlist.index) );
				}
				//Remove the current song from being loaded up
				Mix_FreeMusic(currentSound);
				//Increment the playlist index and get ready to play the next sound
				playlist.index++;
			}
			//Once the playlist is complete, here's what we do next
			if(onCompletion == "end")
			{
				return;
			}
			else if(onCompletion == "loop")
			{
				playlist.index = 0;
				continue;
			}
			else if(onCompletion == "shuffle")
			{
				playlist.index = 0;
				playlist.shuffle();
				continue;
			}
			else
			{
				return;
			}
		}
		return;
	}


	/**
	 * @brief Initiates playing through all of an s_soundPlaylist's sounds.
	 * 		  Typically used for sound effects. Plays all of the sounds in the 
	 * 		  playlist with the requested delays between each. 
	 * 
	 * TODO - This function has been edited so that it does not pass a playlist by reference. This is so we can play playlists of
	 * 		  sound effects on detached threads and copy playlists of soundeffects initialized in stack frames instead of potentially
	 * 		  coming into a memory conflict of playing the same playlist by reference while it's already playing (remember the error
	 * 		  of combat sound effects throwing memory error when we mashed through combat)
	 */
	inline 	void playPlaylist(	SoundPlaylist playlist	)
	{
		playlist.status = "playing";
		std::string categoryName; //The category of the sound that is currently playing
		std::string soundName; //The name of the sound that is currently playing

		//For each sound in the playlist
		for(auto & sound : playlist.sounds)
		{
			categoryName = std::get<0>(playlist.sounds[playlist.index]);
			soundName = std::get<1>(playlist.sounds[playlist.index]);
			//Before we play the track, if we have a preTrackDelay, delay here
			if( playlist.preTrackDelays.contains(playlist.index) )
			{
				SDL_Delay( playlist.preTrackDelays.at(playlist.index) );
			}
			//Play the sound!
			playSound( categoryName, soundName );
			//When the track finishes playing, if we have a postTrackDelay, delay here
			if( playlist.postTrackDelays.contains(playlist.index) )
			{
				SDL_Delay( playlist.postTrackDelays.at(playlist.index) );
			}
			playlist.index++;
		}

		return;
	}
}


/**
 * @brief Initializes the SDL and SDL mixer libraries so they can be interfaced with.
 * @return True if initialization was successful, false otherwise
*/
inline bool initSound()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
	{
		stevensSound::ErrorHandler::setError(stevensSound::ErrorLevel::CRITICAL,
			"SDL could not initialize! SDL Error: " + std::string(SDL_GetError()),
			"initSound");
		return false;
	}
	//Initialize SDL_mixer
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
	{
		stevensSound::ErrorHandler::setError(stevensSound::ErrorLevel::CRITICAL,
			"SDL_mixer could not initialize! SDL_mixer Error: " + std::string(Mix_GetError()),
			"initSound");
		SDL_Quit();
		return false;
	}

	Mix_ChannelFinished(stevensSound_channelFinishedCallback);

	stevensSound::ErrorHandler::setError(stevensSound::ErrorLevel::INFO,
		"SDL and SDL_mixer initialized successfully",
		"initSound");
	return true;
}


/**
 * @brief Closes the SDL and SDL mixer libraries.
*/
inline void closeSound()
{
	// Step 1: Disable callback functions 
	Mix_ChannelFinished(nullptr);
	Mix_HookMusicFinished(nullptr);

	// Step 2: Halt all audio
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

	// Step 3: Free resources
    stevensSound::freeChunks();
	stevensSound::freeMixChunks();

	//Exit the framework
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();
}

#endif // STEVENS_SOUND_HPP