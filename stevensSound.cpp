/**
 * stevensSound.cpp
 * Implementation file for stevensSound library
 * Migrated from header-only to compiled library for faster build times
 */

#include "stevensSound.hpp"

// Global variable definitions
namespace stevensSound {
	std::unordered_map<std::string, std::unordered_map<std::string, Sound> > sounds;
	std::unordered_map<std::string, std::unordered_map<std::string, Music> > music;
	std::unordered_map<std::string, SoundPlaylist> playlists;
	std::unordered_map<std::string, PlaybackController> soundControllers;
}

// Internal command-queue state — owned by this file, not part of the public API
namespace {
	std::queue<stevensSound::AudioCommand> commandQueue;
	std::mutex                             commandMutex;
	std::mutex                             stopMutex;
	std::condition_variable                stopCV;
	bool                                   musicStopped = true;
}

/***** Global functions (outside namespace) *****/

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


/***** stevensSound namespace functions *****/

namespace stevensSound
{
	/**
	 * @brief Returns a std::string containing the version information of SDL and SDL_mixer.
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
	 * @brief Factory function to create a Sound object and load its Mix_Chunks into memory
	 */
	Sound createSound(	std::string name,
						std::string type,
						std::string controllerId,
						const char* filePath,
						std::vector<const char*> variantFilePaths	)
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
	 */
	Music createMusic(	std::string name,
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
	 */
	std::unordered_map<std::string, Sound> loadSoundData(	std::unordered_map<std::string, const char *> soundNamesnPaths,
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
	 */
	std::unordered_map<std::string, Music> loadMusicData(	std::unordered_map<std::string, const char *> musicNamesnPaths,
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
	 * @brief Initializes the sound controllers and loads sound data into this library's sounds map.
	 * 		  Must be called before using this library.
	 */
	bool init(	std::unordered_map<std::string, std::unordered_map<std::string, const char *> > soundsParam    )
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
	 * @brief Used to free all of the chunks stored in memory during shutdown of stevensSound library and SDL Mixer.
	 */
	void freeMixChunks()
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
	bool soundsContains(	const std::string & category,
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
	 */
	void playSound(	const std::string & category,
					const std::string & soundName,
					const std::string & whenChannelsBusy	)
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


	void playSound_detached(	const std::string & category,
								const std::string & soundName,
								const std::string & whenChannelsBusy	)
	{
		std::thread soundThread = std::thread(	playSound,
												category,
												soundName,
												whenChannelsBusy );
		soundThread.detach();
	}


	/**
	 * Creates an s_soundPlaylist object with the given parameters and stores it in the playlists map under a given name.
	 */
	SoundPlaylist createMusicPlaylist(	std::string playlistName,
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


	SoundPlaylist createSoundPlaylist(	std::string playlistName,
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
	 * @brief Pushes a SwitchPlaylist command to the audio thread. Fire-and-forget — returns immediately.
	 */
	void switchMusicPlaylist(	const std::string & switchToPlaylist,
								PlaylistSwitchOptions options			)
	{
		if( !playlists.contains( switchToPlaylist ) )
		{
			ErrorHandler::setError(ErrorLevel::ERROR,
				"Playlist \"" + switchToPlaylist + "\" does not exist",
				"switchMusicPlaylist");
			return;
		}
		std::lock_guard<std::mutex> lock( commandMutex );
		commandQueue.push({ AudioCommandType::SwitchPlaylist, switchToPlaylist, options });
	}


	/**
	 * @brief Pushes a Stop command and blocks until the audio thread acknowledges it.
	 */
	void stopMusicPlaylist()
	{
		{
			std::lock_guard<std::mutex> lock( stopMutex );
			musicStopped = false;
		}
		{
			std::lock_guard<std::mutex> lock( commandMutex );
			commandQueue.push({ AudioCommandType::Stop });
		}
		std::unique_lock<std::mutex> lock( stopMutex );
		stopCV.wait( lock, []{ return musicStopped; } );
	}


	/**
	 * @brief Pushes a SetMusicVolume command so the audio thread applies the new volume.
	 *        Call after updating soundControllers["music"].volume.
	 */
	void setMusicVolume( float volume )
	{
		soundControllers["music"].volume = volume;
		std::lock_guard<std::mutex> lock( commandMutex );
		commandQueue.push({ AudioCommandType::SetMusicVolume });
	}


	/**
	 * @brief Updates the SFX volume. Applied per-chunk at playback time; no audio-thread command needed.
	 */
	void setSfxVolume( float volume )
	{
		soundControllers["sfx"].volume = volume;
	}


	/**
	 * @brief Plays a music playlist on the calling thread (run this on a dedicated std::thread).
	 *        Drains the command queue each poll tick to handle switches, stops, and volume changes.
	 */
	void playMusicPlaylist(	SoundPlaylist & playlist,
							std::string onCompletion)
	{
		playlist.status  = "playing";
		int        pendingFadeInMs = 0; // Carries fade-in duration across a SwitchPlaylist; declared
		Mix_Music* currentSound    = nullptr; // before the label so both survive the goto

		beginPlaylist:
		while(true)
		{
			while(playlist.index < (int)playlist.sounds.size())
			{
				std::string categoryName = std::get<0>(playlist.sounds[playlist.index]);
				std::string soundName    = std::get<1>(playlist.sounds[playlist.index]);

				if( playlist.preTrackDelays.contains(playlist.index) )
					SDL_Delay( playlist.preTrackDelays.at(playlist.index) );

				Music& musicToPlay = music[categoryName][soundName];
				if( !musicToPlay.isValid() )
				{
					SDL_ClearError();
					playlist.index++;
					continue;
				}

				currentSound = musicToPlay.musicData.music;

				// Always set volume before starting so the loaded userData value is applied immediately
				Mix_VolumeMusic((int)(128.0f * soundControllers[playlist.controllerId].volume));
				if( pendingFadeInMs > 0 )
				{
					Mix_FadeInMusic(currentSound, 1, pendingFadeInMs);
					pendingFadeInMs = 0;
				}
				else
				{
					Mix_PlayMusic(currentSound, 1);
				}
				if( playlist.trackPosition > 0.0 )
				{
					Mix_SetMusicPosition(playlist.trackPosition);
					playlist.trackPosition = 0.0;
				}

				while( Mix_PlayingMusic() )
				{
					bool switchRequested = false;
					bool stopRequested   = false;

					{
						std::lock_guard<std::mutex> lock( commandMutex );
						while( !commandQueue.empty() )
						{
							AudioCommand cmd = std::move( commandQueue.front() );
							commandQueue.pop();

							switch( cmd.type )
							{
							case AudioCommandType::SwitchPlaylist:
							{
								double pos = Mix_GetMusicPosition(currentSound);
								if( pos >= 0.0 ) playlist.trackPosition = pos;
								Mix_HaltMusic();
								currentSound = nullptr;
								// Save current state back to its named slot, then load the new playlist
								playlists.at( playlist.getName() ) = playlist;
								playlist        = playlists.at( cmd.playlistName );
								playlist.status = "playing";
								pendingFadeInMs = cmd.options.fadeInMs;
								switchRequested = true;
								break;
							}
							case AudioCommandType::Stop:
								Mix_HaltMusic();
								currentSound = nullptr;
								stopRequested = true;
								break;
							case AudioCommandType::SetMusicVolume:
							{
								float vol = soundControllers[playlist.controllerId].volume;
								Mix_VolumeMusic((int)(128.0f * vol));
								if( vol == 0.0f )
								{
									Mix_PauseMusic();
									playlist.status = "paused";
								}
								else if( playlist.status == "paused" )
								{
									Mix_ResumeMusic();
									playlist.status = "playing";
								}
								break;
							}
							}
							if( switchRequested || stopRequested ) break;
						}
					}

					if( stopRequested )
					{
						{
							std::lock_guard<std::mutex> stopLock( stopMutex );
							musicStopped = true;
						}
						stopCV.notify_one();
						return;
					}
					if( switchRequested )
						goto beginPlaylist;

					SDL_Delay(100);
				}

				if( playlist.postTrackDelays.contains(playlist.index) )
					SDL_Delay( playlist.postTrackDelays.at(playlist.index) );

				// Note: do NOT call Mix_FreeMusic here — music is cached in the global map for reuse
				playlist.index++;
			}

			if(onCompletion == "loop")
			{
				playlist.index = 0;
			}
			else if(onCompletion == "shuffle")
			{
				playlist.index = 0;
				playlist.shuffle();
			}
			else
			{
				return;
			}
		}
	}


	/**
	 * @brief Initiates playing through all of an s_soundPlaylist's sounds.
	 * 		  Typically used for sound effects. Plays all of the sounds in the
	 * 		  playlist with the requested delays between each.
	 */
	void playPlaylist(	SoundPlaylist playlist	)
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

} // namespace stevensSound


/***** Global functions at file scope (outside namespace) *****/

/**
 * @brief Initializes the SDL and SDL mixer libraries so they can be interfaced with.
 */
bool initSound()
{
	//Initialize SDL
	if( SDL_Init( SDL_INIT_AUDIO ) < 0 )
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
	stevensSound::freeMixChunks();

	//Exit the framework
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();
}
