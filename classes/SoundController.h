/**
 * @file SoundController.h
 * @brief Controller for managing sound volume settings
 */

#ifndef STEVENSSOUND_SOUNDCONTROLLER_H
#define STEVENSSOUND_SOUNDCONTROLLER_H

#include <string>

namespace stevensSound
{

/**
 * @brief Controls volume and playback settings for sounds
 */
class SoundController
{
public:
	/*** Member variables ***/
	std::string id;     ///< The identifying string of the sound controller
	float volume;       ///< Volume level (0.0 = silent, 1.0 = 100% volume)

	/*** Constructors ***/
	SoundController()
		: id("default")
		, volume(1.0f)
	{
	}

	SoundController(std::string idParam, float volumeParam)
		: id(idParam)
		, volume(volumeParam)
	{
	}
};

} // namespace stevensSound

#endif // STEVENSSOUND_SOUNDCONTROLLER_H
