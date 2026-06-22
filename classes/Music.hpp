#pragma once

#include <string>
#include "Mix_MusicData.h"
#include "ErrorHandler.hpp"

namespace stevensSound
{

/**
 *  An object used in the stevensSound library which represents music and its associated data
 *  within code. Music uses Mix_Music for streaming playback.
 */
class Music
{
    public:
        /*** Member variables ***/
        std::string  name;
        std::string  type;
        std::string controllerId;

        Mix_MusicData musicData;


        /*** Constructors ***/
        Music()
        {
            name = "default";
            type = "default";
            controllerId = "default";
            musicData = Mix_MusicData();
        }


        Music(  std::string nameParam,
                std::string typeParam,
                std::string controllerIdParam   )
        {
            name = nameParam;
            type = typeParam;
            controllerId = controllerIdParam;
            musicData = Mix_MusicData();
        }


        /*** Methods ***/
        bool isValid() const
        {
            if (musicData.music == nullptr)
            {
                ErrorHandler::setError(ErrorLevel::ERROR,
                    "Music handle is null for: " + name + " (" + musicData.filePath + ")",
                    "Music::isValid");
                return false;
            }
            return true;
        }

        void freeMusic()
        {
            musicData.free();
        }


    private:
};

} // namespace stevensSound
