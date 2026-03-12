#ifndef STEVENS_SOUND_SOUND_HPP
#define STEVENS_SOUND_SOUND_HPP

#include <string>

/**
 *  An object used in the stevensSound library which represents a specific sound and its associated data
 *  within code.
 */
class Sound
{
    public:
        /*** Member variables ***/
        std::string  name;       //The name of the sound - Will also be used as the key to reference this object in the sounds map
        std::string  type;       //The type of sound. This controls the location the sound will be stored in the sounds map.
        std::string controllerId; //The id of the sound controller of this sound.

        Mix_ChunkData mainChunkData;  //The primary sound chunk with file path and Mix_Chunk
        std::vector<Mix_ChunkData> variantChunkData;  //Pitch-shifted variants for anti-fatigue (if any)


        /*** Constructors ***/
        Sound()
        {
            name = "default";
            type = "default";
            controllerId = "default";
            mainChunkData = Mix_ChunkData();
            variantChunkData = {};
        }


        Sound(  std::string nameParam,
                std::string typeParam,
                std::string controllerIdParam   )
        {
            name = nameParam;
            type = typeParam;
            controllerId = controllerIdParam;
            mainChunkData = Mix_ChunkData();
            variantChunkData = {};
        }


        /*** Methods ***/
        bool hasVariants() const
        {
            return !variantChunkData.empty();
        }

        /**
         * @brief Validates that this Sound object's Mix_Chunk handle is safe to use
         *
         * Checks that the main chunk pointer is not null and is valid for playback.
         * If validation fails, logs an error with the sound's name for debugging.
         *
         * @return true if the sound is valid and safe to play, false otherwise
         */
        bool isValid() const
        {
            if (mainChunkData.chunk == nullptr)
            {
                ErrorHandler::setError(ErrorLevel::ERROR,
                    "Sound chunk is null for: " + name + " (" + mainChunkData.filePath + ")",
                    "Sound::isValid");
                return false;
            }
            return true;
        }

        /**
         * @brief Get a random chunk to play (anti-fatigue: randomly selects from main + variants)
         * @return Mix_Chunk* pointer to play
         */
        Mix_Chunk* getChunkToPlay()
        {
            if( !hasVariants() )
            {
                //No variants, just return the main chunk
                return mainChunkData.chunk;
            }

            //Build vector of all available chunks (main + variants)
            std::vector<Mix_Chunk*> availableChunks;
            availableChunks.push_back( mainChunkData.chunk );
            for( auto& variantData : variantChunkData )
            {
                availableChunks.push_back( variantData.chunk );
            }

            //Randomly select one
            return stevensVectorLib::getRandomElement( availableChunks );
        }

        /**
         * @brief Free all Mix_Chunks owned by this sound data object
         *
         * WARNING: Should ONLY be called during program shutdown in cleanUp().
         * Do NOT call this during normal playback!
         */
        void freeAllChunks()
        {
            //Free main chunk
            mainChunkData.free();

            //Free all variant chunks
            for( auto & variantData : variantChunkData )
            {
                variantData.free();
            }
        }


    private:
};

#endif // STEVENS_SOUND_SOUND_HPP
