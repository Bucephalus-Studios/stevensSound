#pragma once

#include <string>

namespace stevensSound
{

/**
 * Controls playback settings (volume, etc.) for sounds and music in stevensSound library
 */
class PlaybackController
{
    public:
        /*** Member variables ***/
        std::string id;
        float volume;


        /*** Constructors ***/
        PlaybackController()
        {
            id = "default";
            volume = 1;
        }


        PlaybackController( std::string idParam,
                            float volumeParam    )
        {
            id = idParam;
            volume = volumeParam;
        }


    private:
};

} // namespace stevensSound
