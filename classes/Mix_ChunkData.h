#pragma once

#include <string>
#include <iostream>

#if defined(__linux__) || defined(_WIN32)
    #include <SDL2/SDL_mixer.h>
#endif

/**
 * A wrapper object that pairs a sound file path with its loaded Mix_Chunk.
 * Allows for flexible memory management - chunks can be loaded/freed on demand.
 */

class Mix_ChunkData
{
    public:
        /*** Member variables ***/
        std::string filePath;    //Path to the sound file
        Mix_Chunk* chunk;        //The loaded Mix_Chunk (nullptr if not loaded)


        /*** Constructors ***/
        Mix_ChunkData()
        {
            filePath = "";
            chunk = nullptr;
        }


        Mix_ChunkData( const char * filePathParam )
        {
            filePath = filePathParam;
            chunk = nullptr;
        }


        /*** Methods ***/
        /**
         * @brief Load the Mix_Chunk from the file path into memory
         * @return True if successful, false otherwise
         */
        bool load()
        {
            //If already loaded, don't reload
            if( chunk != nullptr )
            {
                return true;
            }

            //Load the chunk from file
            chunk = Mix_LoadWAV( filePath.c_str() );
            if( chunk == nullptr )
            {
                std::cerr << "Failed to load sound from " << filePath << ": " << Mix_GetError() << std::endl;
                return false;
            }

            return true;
        }


        /**
         * @brief Free the Mix_Chunk from memory
         */
        void free()
        {
            if( chunk != nullptr )
            {
                Mix_FreeChunk( chunk );
                chunk = nullptr;
            }
        }


        /**
         * @brief Check if this chunk is currently loaded in memory
         */
        bool isLoaded() const
        {
            return chunk != nullptr;
        }


    private:
};
