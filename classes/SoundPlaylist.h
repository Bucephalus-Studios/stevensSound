/**
 * @file SoundPlaylist.h
 * @brief Playlist structure for managing ordered sequences of sounds
 */

#ifndef STEVENSSOUND_SOUNDPLAYLIST_H
#define STEVENSSOUND_SOUNDPLAYLIST_H

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <iostream>

namespace stevensSound
{

// Forward declaration
class SoundData;

/**
 * @brief Holds ordered lists of sound references for playlist functionality
 *
 * Playlists are used for the playPlaylist() function in the stevensSound library,
 * where a playlist can be played sequentially.
 */
class SoundPlaylist
{
public:
	/*** Member variables ***/
	std::string name;                                          ///< The identifying name of the playlist
	std::vector<std::tuple<std::string, std::string>> sounds; ///< Keys of sounds (category, soundName) in playlist order
	int index;                                                 ///< Index of the currently playing song
	std::string controllerId;                                  ///< The sound controller which this playlist is assigned to
	std::string status;                                        ///< Current status: "stopped", "paused", or "playing"
	std::unordered_map<int, int> preTrackDelays;              ///< Delay in ms before each track
	std::unordered_map<int, int> postTrackDelays;             ///< Delay in ms after each track

	/*** Constructors ***/

	/// Default constructor
	SoundPlaylist()
		: name("unnamed playlist")
		, sounds{}
		, index(0)
		, controllerId("default")
		, status("stopped")
		, preTrackDelays{}
		, postTrackDelays{}
	{
	}

	/**
	 * @brief Create a playlist from specified sound categories and track order
	 *
	 * @param nameParam Name of the playlist
	 * @param sourceSounds The map of all sounds from the stevensSound library
	 * @param soundCategories Categories to pull tracks from
	 * @param trackOrder Names of tracks in desired order
	 * @param controllerIdParam Sound controller ID for volume control
	 * @param shuffleFill If true, fill remaining playlist with random sounds from categories
	 * @param preTrackDelaysParam Delays before tracks (map: index -> milliseconds)
	 * @param postTrackDelaysParam Delays after tracks (map: index -> milliseconds)
	 *
	 * @note If multiple sounds have the same name in different categories, the sound
	 *       from the first category in soundCategories will be used.
	 */
	SoundPlaylist(const std::string& nameParam,
	              std::unordered_map<std::string, std::unordered_map<std::string, SoundData>>& sourceSounds,
	              std::vector<std::string> soundCategories,
	              std::vector<std::string> trackOrder,
	              std::string controllerIdParam,
	              bool shuffleFill = false,
	              std::unordered_map<int, int> preTrackDelaysParam = {},
	              std::unordered_map<int, int> postTrackDelaysParam = {})
		: name(nameParam)
		, sounds{}
		, index(0)
		, controllerId(controllerIdParam)
		, status("stopped")
		, preTrackDelays(preTrackDelaysParam)
		, postTrackDelays(postTrackDelaysParam)
	{
		/*** Create the playlist ***/

		// Create a working copy of sounds we can modify
		std::unordered_map<std::string, std::unordered_map<std::string, SoundData>> soundsToAdd;
		for (const auto& category : soundCategories)
		{
			soundsToAdd[category] = sourceSounds[category];
		}

		// For every sound named in trackOrder, search through sound categories to find it
		bool foundTrack = false;
		for (const auto& trackName : trackOrder)
		{
			for (const auto& category : soundCategories)
			{
				// Can we find the requested track in this sound category?
				if (soundsToAdd[category].contains(trackName))
				{
					// Yes, add it to the playlist
					sounds.push_back(std::make_tuple(category, trackName));
					// Remove it from the soundsToAdd map
					soundsToAdd[category].erase(trackName);
					foundTrack = true;
					break;
				}
			}

			// Check if we found the track. If not, send an error
			if (!foundTrack)
			{
				std::cerr << "stevensSound library error: createPlaylist() : Unable to find requested track '"
				          << trackName << "' in sounds map.\n";
			}
			else
			{
				foundTrack = false;
			}
		}

		// Shuffle fill if requested
		if (shuffleFill)
		{
			// Until we have no more sounds left to add, pick random sound from random category
			while (!soundsToAdd.empty())
			{
				// Get a random category
				auto category_it = soundsToAdd.begin();
				std::advance(category_it, rand() % soundsToAdd.size());
				std::string random_category = category_it->first;

				// Get a random sound from that category
				auto sound_it = soundsToAdd[random_category].begin();
				std::advance(sound_it, rand() % soundsToAdd[random_category].size());
				std::string random_sound = sound_it->first;

				// Add the sound to the playlist
				sounds.push_back(std::make_tuple(random_category, random_sound));

				// Erase the sound from the category
				soundsToAdd[random_category].erase(random_sound);

				// If the category is now empty, erase it
				if (soundsToAdd[random_category].empty())
				{
					soundsToAdd.erase(random_category);
				}
			}
		}
	}

	/*** Methods ***/

	/**
	 * @brief Shuffle the order of sounds in this playlist
	 */
	void shuffle()
	{
		std::shuffle(sounds.begin(), sounds.end(), std::default_random_engine());
	}

	/**
	 * @brief Get the name of this playlist
	 * @return The playlist name
	 */
	std::string getName() const
	{
		return name;
	}
};

} // namespace stevensSound

#endif // STEVENSSOUND_SOUNDPLAYLIST_H
