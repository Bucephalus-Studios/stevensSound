/**
 * @file SoundData.h
 * @brief Data structure representing a sound and its associated metadata
 */

#pragma once

#include <string>

namespace stevensSound
{

/**
 * @brief Represents a specific sound and its associated data
 */
class SoundData
{
public:
	/*** Member variables ***/
	std::string  name;         ///< The name of the sound - used as the key in the sounds map
	const char * filePath;     ///< The path to the sound file from the executable's location
	std::string  type;         ///< The type/category of sound
	std::string controllerId;  ///< The ID of the sound controller for this sound

	/*** Constructors ***/
	SoundData()
		: name("default")
		, filePath("no filepath defined for this SoundData object")
		, type("default")
		, controllerId("default")
	{
	}

	SoundData(std::string nameParam,
			  const char* filePathParam,
			  std::string typeParam,
			  std::string controllerIdParam)
		: name(nameParam)
		, filePath(filePathParam)
		, type(typeParam)
		, controllerId(controllerIdParam)
	{
	}
};

} // namespace stevensSound
