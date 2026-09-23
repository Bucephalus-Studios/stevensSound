#pragma once

#include <string>
#include <vector>
#include "Mix_ChunkData.h"
#include <spdlog/spdlog.h>
#include <stevensVectorLib.hpp>

namespace stevensSound
{

/**
 *  An object used in the stevensSound library which represents a specific sound and its associated data
 *  within code.
 */
class Sound
{
    public:
        /*** Member variables ***/
        std::string  name;
        std::string  type;
        std::string controllerId;

        Mix_ChunkData mainChunkData;
        std::vector<Mix_ChunkData> variantChunkData;

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

        bool isValid() const
        {
            if (mainChunkData.chunk == nullptr)
            {
                spdlog::error("stevensSound: Sound::isValid: Sound chunk is null for: {} ({})", name, mainChunkData.filePath);
                return false;
            }
            return true;
        }

        Mix_Chunk* getChunkToPlay()
        {
            if( !hasVariants() )
            {
                return mainChunkData.chunk;
            }

            std::vector<Mix_Chunk*> availableChunks;
            availableChunks.push_back( mainChunkData.chunk );
            for( auto& variantData : variantChunkData )
            {
                availableChunks.push_back( variantData.chunk );
            }

            return stevensVectorLib::getRandomElement( availableChunks );
        }

        void freeAllChunks()
        {
            mainChunkData.free();
            for( auto & variantData : variantChunkData )
            {
                variantData.free();
            }
        }

    private:
};

} // namespace stevensSound
