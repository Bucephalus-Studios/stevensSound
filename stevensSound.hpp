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

//Custom libraries used here
// #include "libraries/stevensSetLib.h"

//Include classes for the library
#include "classes/s_soundData.h"
#include "classes/s_soundController.h"
#include "classes/s_soundPlaylist.h"
#include "classes/s_errorHandler.h"

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


/***** Global functions and variables used for managing Mix_Chunk memory *****/
static std::mutex stevensSound_chunkMutex;
static std::unordered_set< Mix_Chunk* > stevensSound_chunkPool;


/**
 * @brief Checks to see if a given Mix_Chunk is actively playing on a channel
 */
bool stevensSound_isChunkPlaying( Mix_Chunk * chunk )
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
void stevensSound_channelFinishedCallback( const int channel )
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


namespace stevensSound
{
	/*** Variables ***/
	std::unordered_map<std::string, std::unordered_map<std::string, s_soundData> > sounds; //Container of all sounds
	std::unordered_map<std::string, s_soundPlaylist> playlists; //Contains all of the playlists created with the stevensSound library
	std::unordered_map<std::string, s_soundController> soundControllers; //Container of all sound controllers. Controls volume and playback settings for sounds.
	std::unordered_map<std::string, Mix_Chunk*> persistentChunks; //A map containing the addresses of Mix_Chunks we want to keep stored in memory.
																  //The benefit of this is we don't have to load these sounds into memory and free them
																  //on each use.
	bool audioChange; //Bool used to control when currently audio playback is modified in any way


	/*** Methods ***/
	/**
	 * @brief Returns a string containing the version information of SDL and SDL_mixer.
	 */
	std::string getSDLVersionInfo()
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
	 * @brief Given an unordered_map with key-value pairs of sound names and file paths to the sounds respectively and 
	 * a type of sound, return an unordered_map with key value pairs of sound names and s_soundData objects
	 * containing data about the sounds. 
	 * 
	 * Parameters:
	 *  unordered_map<std::string, const char *> soundNamesnPaths - A map of key-value pairs of sound names and their
	 *                                                              file paths relative to the executable file location
	 *                                                              respectively.
	 *  std::string soundType - The type of sound to assign to all of the newly constructed s_soundData objects.
	 * 	std::string controllerId - Id of the controller to which to assign each of the newly loaded sounds to.
	 * 
	 * Returns:
	 *  unordered_map<std::string, s_soundData> - A map of sound objects where the key-value pairs are of the sound name
	 *                                            and the data object containing all necessary information about the 
	 *                                            sound needed for it to be used in this library respectively.
	 *  
	*/
	std::unordered_map<std::string, s_soundData> loadSoundData(	std::unordered_map<std::string, const char *> soundNamesnPaths,
																std::string soundType,
																std::string controllerId  )
	{
		std::unordered_map<std::string, s_soundData> soundDataMap = {};
		s_soundData soundData;

		//Iterate through all of the sound names and paths
		for(auto & [soundName,path] : soundNamesnPaths)
		{
			//Convert each key-value pair into an s_soundData object that is an entry of soundDataMap
			soundData = s_soundData(soundName, path, soundType, controllerId);
			soundDataMap[soundName] = soundData;
		}
		//Load the sound files in memory to test if they're able to load
		std::unordered_map<std::string, s_soundData>::iterator it;
		for (it = soundDataMap.begin(); it != soundDataMap.end(); it++)
		{
			Mix_Chunk * chunk = Mix_LoadWAV(it->second.filePath);
			if( chunk == NULL)
			{
				//Print an error to cerr if we're unable to load a certain file
				std::cerr << soundType + " " << it->second.name << " was unable to load!" << Mix_GetError() << std::endl;
			}
			else
			{
				Mix_FreeChunk(chunk);
			}
		}

		return soundDataMap;
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
	bool init(	std::unordered_map<std::string, std::unordered_map<std::string, const char *> > soundsParam    )
	{
		//Create music, sfx, and default sound controllers
		soundControllers = {	{"music",	s_soundController("music",1)},
								{"sfx",		s_soundController("sfx",1)},
								{"default",	s_soundController()}		};

		//Initialize each type of sound we passed in as a parameter
		sounds = {};
		for(auto & [soundType, soundMap] : soundsParam)
		{
			sounds[soundType] = loadSoundData(soundMap, soundType, soundType);
		}

		stevensSound::audioChange = false;

		int channels = Mix_AllocateChannels(16);
		if (channels != 16)
		{
			ErrorHandler::setError(ErrorLevel::WARNING,
				"Failed to allocate 16 channels, got " + std::to_string(channels),
				"init");
		}

		//Initialize our playlists container to be empty other than having an empty "currently playing" playlist
		playlists.emplace( "currently playing", s_soundPlaylist() );

		ErrorHandler::setError(ErrorLevel::INFO, "stevensSound initialized successfully", "init");
		return true;
	}


	/**
	 * @brief Frees all of the chunks that we've allocated in non-persistent storage
	 */
	void freeChunks()
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
	 * @brief Used to free all of the persistent chunks stored in memory during shutdown of stevensSound library and SDL Mixer.
	 */
	void freePersistentChunks()
	{
		std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);

		//Free all chunks in the persistentChunks map
		for( const auto & [id, chunkPtr] : stevensSound::persistentChunks )
		{
			Mix_FreeChunk(chunkPtr);
		}

		//Clear the map
		stevensSound::persistentChunks.clear();
	}


	/**
	 * @brief Returns true if a sound is found to be persistently stored within the persistentChunks map and 
	 * 		  false otherwise.
	 * 
	 * @param category The category the sound is stored under in the sounds map
	 * @param soundName The identifying name of the sound
	 */
	bool isPersistentlyStored(	const std::string & category,
								const std::string & soundName	)
	{
		return stevensSound::persistentChunks.contains( category + "/" + soundName );
	}


	/**
	 * @brief Returns true if we have an entry in the sounds map with a matching category and
	 * 		  soundName.
	 */
	bool soundsContains(	const std::string & category,
							const std::string & soundName	)
	{
		if( stevensSound::sounds.contains( category ) ) 
		{
			return stevensSound::sounds.at( category ).contains( soundName );
		}
		return false;
	}


	/**
	 * @brief Store a sound from the sounds map in persistent memory storage to avoid reloading the sound.
	 * 
	 * @param cateogry The category the soudn is stored under in the sounds map
	 * @param soundName THe identifying name of the sound
	 */
	void storePersistentSound(	const std::string & category,
								const std::string & soundName	)
	{
		//Check to see if the sound exists in sounds
		if( !stevensSound::soundsContains( category, soundName ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Could not find sound with category \"" + category + "\" and name \"" + soundName + "\"",
				"storePersistentSound");
			return;
		}
		//Is the sound already stored persistently in persistentChunks?
		if( stevensSound::isPersistentlyStored( category, soundName ) )
		{
			//If so, free the chunk already stored there and continue
			Mix_FreeChunk( stevensSound::persistentChunks.at( category + "/" + soundName ) );
		}

		//Load the sound into memory
		Mix_Chunk* sound = Mix_LoadWAV(sounds[category][soundName].filePath);

		//Create an entry for the sound as "{category}/{soundName}" and set the value equal to the chunk pointer
		stevensSound::persistentChunks[ category + "/" + soundName ] = sound;
	}


	/**
	 * @brief Free a sound from the persistentChunks map from memory.
	 * 
	 * @param cateogry The category the soudn is stored under in the sounds map
	 * @param soundName THe identifying name of the sound
	 */
	void freePersistentSound(	const std::string & category,
								const std::string & soundName	)
	{
		//Is the sound stored persistently in persistentChunks?
		if( !stevensSound::isPersistentlyStored( category, soundName ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Sound with category \"" + category + "\" and name \"" + soundName + "\" is not persistently stored",
				"freePersistentSound");
			return;
		}	

		//Otherwise, free the sound from memory
		Mix_FreeChunk( stevensSound::persistentChunks.at( category + "/" + soundName ) );

		//Erase the entry for the soudn in persistentChunks
		stevensSound::persistentChunks.erase( category + "/" + soundName );
	}


	/**
	 * @brief Play a sound that exists in persistent memory which is tracked in the persistentChunks map
	 * 
	 * @param category The category in the sounds map that the sound is stored under
	 * @param soundName The identifying name of the sound
	 */
	void playPersistentSound(	const std::string & category,
								const std::string & soundName,
								const std::string & whenChannelsBusy = "return"	)
	{
		//stevensFileLib::appendToFile("errorLog.txt", "playing persistent sound!\n");

		//Get the persistently stored chunk
		Mix_Chunk* sound = persistentChunks.at( category + "/" + soundName );

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
	void playSound(	const std::string & category,
					const std::string & soundName,
					const std::string & whenChannelsBusy = "return"	)
	{
		//Check: Is the category and soundName combo a persistently loaded sound?
		if( stevensSound::isPersistentlyStored( category, soundName ) )
		{
			//If so, play the preloaded sound
			stevensSound::playPersistentSound( category, soundName, whenChannelsBusy );
			return;
		}
		//Also check: is the category and soundName combo an existing sound in the 2D sounds map?
		else if( !stevensSound::soundsContains( category, soundName ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Requested to play sound with category \"" + category + "\" and name \"" + soundName + "\", but it does not exist",
				"playSound");
			return;
		}

		//Load the sound file into memory
		Mix_Chunk* sound = Mix_LoadWAV(sounds[category][soundName].filePath);
		//If the sound doesn't load, return
		if( !sound )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Failed to load sound file: " + std::string(sounds[category][soundName].filePath) + " - " + Mix_GetError(),
				"playSound");
			return;
		}

		// Add to pool before playing (avoid race condition)
		{
			std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);
			stevensSound_chunkPool.insert(sound);
		}

		// Get audio effects for this sound
		AudioEffects effects = AudioEffectsManager::getEffects(category, soundName);

		// Apply random pitch variation if enabled
		if (effects.randomizePitch)
		{
			float randomVar = AudioEffectsManager::getRandomPitchVariation(effects.randomRange);
			effects.pitchVariation += randomVar;
		}

		//Control the playback with effects
		float baseVolume = soundControllers[sounds[category][soundName].controllerId].volume;
		float effectiveVolume = AudioEffectsManager::calculateEffectiveVolume(baseVolume, effects);
		Mix_VolumeChunk(sound, (int)round(128 * effectiveVolume));

		/***** PLAY THE SOUND *****/
		int channel = -1;
		if( whenChannelsBusy == "return" )
		{
			//Try to find an open channel
			SDL_LockAudioDevice(1);
			channel = Mix_PlayChannel( -1, sound, 0 );
			SDL_UnlockAudioDevice(1);

			// Apply audio effects to the channel
			if (channel != -1)
			{
				AudioEffectsManager::applyEffectsToChannel(channel, effects);
			}
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

			// Apply audio effects to the channel
			if (channel != -1)
			{
				AudioEffectsManager::applyEffectsToChannel(channel, effects);
			}
		}
		else //steal channel
		{
			//TODO FIND CHANNEL TO STEAL
			void(0);
		}

		//If Mix_PlayChannel returns -1, the sound could not be played. Free the chunk and return the function
		if (channel == -1) 
		{
			std::lock_guard<std::mutex> lock(stevensSound_chunkMutex);
			// stevensFileLib::appendToFile( "stevensSound.log", "Unable to play sound on channel: " + std::to_string(channel) + "\n" );
			// stevensFileLib::appendToFile( "stevensSound.log", "Mix_PlayChannel error: " + std::string(Mix_GetError()) + "\n" );
			//Remove the chunk from the pool
        	stevensSound_chunkPool.erase(sound);
			// //Add delay to prevent errors of loading and freeing sounds too quickly
			// SDL_Delay(100);
			Mix_FreeChunk(sound);
			return;
		}

		//Fallback: Garbage collect every 25 plays
		static int cleanupCounter = 0;
		if ( ++cleanupCounter >= 25 )
		{
			cleanupCounter = 0;
			stevensSound::freeChunks(); // Forces cleanup of all chunks
		}
	}


	void playSound_detached(	const std::string & category,
								const std::string & soundName,
								const std::string & whenChannelsBusy = "return"	)
	{
		std::thread soundThread = std::thread(	stevensSound::playSound,
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
	void createPlaylist(	std::string playlistName,
							std::string controllerId,
							std::vector<std::string> soundCategoriesUsed,
							std::vector<std::string> trackOrder,
							bool shuffleFill	)
	{
		playlists.emplace( playlistName, s_soundPlaylist(	playlistName,
															sounds,
															soundCategoriesUsed,
															trackOrder,
															controllerId,
															shuffleFill		)	);
	}


	/**
	 * @brief Used to switch the content of the currently playing playlist in the playMusicPlaylist function
	 * 		  on a separate thread.
	 * 
	 * @param switchToPlaylist The playlist that we are switching to start playing the music of
	 */
	void switchMusicPlaylist(	const std::string & switchToPlaylist	)
	{
		//Before we try a switch, make sure the playlist we want to switch to exists
		if( !stevensSound::playlists.contains( switchToPlaylist ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Playlist \"" + switchToPlaylist + "\" does not exist",
				"switchMusicPlaylist");
			return;
		}
		//Also make sure the playlist we are switching from still is stored under the same key in stevensSound::playlists
		if( !stevensSound::playlists.contains( stevensSound::playlists.at("currently playing").getName() ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Currently playing playlist \"" + stevensSound::playlists.at("currently playing").getName() + "\" does not exist in playlists map",
				"switchMusicPlaylist");
			return;
		}

		//Set the currently playing playlist to start the switching process. This will trigger an if statement in playMusicPlaylist()
		stevensSound::playlists.at("currently playing").status = "switching";
        stevensSound::audioChange = true;

		//Wait for the music change to be completed in the playMusicPlaylist() function
		while( stevensSound::audioChange )
		{
			SDL_Delay(10);
		}
		
		//Store the currrent state of the current playlist before switching it
		stevensSound::playlists.at( stevensSound::playlists.at("currently playing").getName()  ) = stevensSound::playlists.at("currently playing");

		//Switch the playlist now
		stevensSound::playlists.at("currently playing") = stevensSound::playlists.at(switchToPlaylist);

		//Start up the music again
		stevensSound::playlists.at("currently playing").status = "playing";
		stevensSound::audioChange = true;
	}


	/**
	 * @brief Stops the currently playing playlist on a separate thread.
	 */
	void stopMusicPlaylist()
	{
		//Stop the current music
		stevensSound::playlists.at("currently playing").status = "stopped";
		stevensSound::audioChange = true;

		//Wait for the music to stop
		while(stevensSound::audioChange)
		{
			SDL_Delay(100);
		}

		return;
	}


	/**
	 * @brief Initiates playing a playlist for music. Typically used on a separate thread than the main one.
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
	void playMusicPlaylist(	s_soundPlaylist & playlist,
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
				//Try to load in the music file
				currentSound = Mix_LoadMUS( stevensSound::sounds[categoryName][soundName].filePath );
				//If we fail to load the sound in, print an error message and go to the next sound in the music playlist
				if (currentSound == nullptr)
				{
					std::string errorMsg = "Mix_LoadMUS failed for: ";
					errorMsg += stevensSound::sounds[categoryName][soundName].filePath;
					errorMsg += " - Error: ";
					errorMsg += Mix_GetError();
					//stevensFileLib::appendToFile("errorLog.txt", errorMsg + "\n");

					SDL_ClearError();  // Clear the error state
					playlist.index++;
					continue;
				}

				//Play the music!
				Mix_PlayMusic(currentSound, 1);

				while( Mix_PlayingMusic() )
				{
					if( stevensSound::audioChange ) //If a change in music settings is detected, apply the change to the current playing music
					{
						/*** Switch the playlist to another ***/
						if (playlist.status == "switching")
						{
							Mix_HaltMusic();
							Mix_FreeMusic(currentSound);
							currentSound = nullptr;
							stevensSound::audioChange = false;
							//Go to the top of this function
							goto beginPlaylist;
						}
						/*** If playlist set to stopped, stop playing this playlist and exit this function ***/
						if	(playlist.status == "stopped")
						{
							Mix_HaltMusic();
							Mix_FreeMusic(currentSound);
							stevensSound::audioChange = false;
							return;
						}
						/*** Apply volume changes ***/
						Mix_VolumeMusic(128 * stevensSound::soundControllers[playlist.controllerId].volume);
						/*** Pausing and unpausing playlist ***/
						if ( stevensSound::soundControllers[playlist.controllerId].volume == 0) //If the volume becomes 0, pause the song
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
						stevensSound::audioChange = false;
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
	void playPlaylist(	s_soundPlaylist playlist	)
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
			stevensSound::playSound( categoryName, soundName );
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
bool initSound()
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
void closeSound()
{
	// Step 1: Disable callback functions 
	Mix_ChannelFinished(nullptr);
	Mix_HookMusicFinished(nullptr);

	// Step 2: Halt all audio
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

	// Step 3: Free resources
    stevensSound::freeChunks();
	stevensSound::freePersistentChunks();

	//Exit the framework
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();
}