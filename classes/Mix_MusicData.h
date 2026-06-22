#pragma once

#include <string>
#include <iostream>

#if defined(__linux__) || defined(_WIN32)
    #include <SDL2/SDL_mixer.h>
#endif

/**
 * A wrapper object that pairs a music file path with its loaded Mix_Music handle.
 * Mix_Music streams audio from disk rather than loading it entirely into memory.
 */

class Mix_MusicData
{
    public:
        /*** Member variables ***/
        std::string filePath;    //Path to the music file
        Mix_Music* music;        //The loaded Mix_Music handle (nullptr if not loaded)


        /*** Constructors ***/
        Mix_MusicData()
        {
            filePath = "";
            music = nullptr;
        }


        Mix_MusicData( const char * filePathParam )
        {
            filePath = filePathParam;
            music = nullptr;
        }


        /*** Methods ***/
        /**
         * @brief Load the Mix_Music handle from the file path
         * @return True if successful, false otherwise
         */
        bool load()
        {
            //If already loaded, don't reload
            if( music != nullptr )
            {
                return true;
            }

            //Load the music handle from file
            music = Mix_LoadMUS( filePath.c_str() );
            if( music == nullptr )
            {
                std::cerr << "Failed to load music from " << filePath << ": " << Mix_GetError() << std::endl;
                return false;
            }

            return true;
        }


        /**
         * @brief Free the Mix_Music handle from memory
         */
        void free()
        {
            if( music != nullptr )
            {
                Mix_FreeMusic( music );
                music = nullptr;
            }
        }


        /**
         * @brief Check if this music handle is currently loaded
         */
        bool isLoaded() const
        {
            return music != nullptr;
        }


    private:
};
