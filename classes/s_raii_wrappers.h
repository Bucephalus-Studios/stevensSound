/**
 * @file s_raii_wrappers.h
 * @brief RAII wrappers for SDL_mixer resources
 * @version 1.0
 * @date 2025-11-14
 *
 * This file provides RAII (Resource Acquisition Is Initialization) wrappers
 * for SDL_mixer resources to ensure automatic cleanup and prevent memory leaks.
 */

#pragma once

#include <SDL2/SDL_mixer.h>
#include <memory>
#include <string>

namespace stevensSound
{
    /**
     * @brief RAII wrapper for Mix_Chunk
     *
     * Automatically loads and frees Mix_Chunk resources.
     * Prevents memory leaks and ensures proper cleanup.
     */
    class MixChunkRAII
    {
    private:
        Mix_Chunk* chunk;
        std::string filepath;
        bool ownsResource;

    public:
        /**
         * @brief Construct from a file path
         * @param path Path to the audio file
         */
        explicit MixChunkRAII(const char* path)
            : chunk(Mix_LoadWAV(path))
            , filepath(path ? path : "")
            , ownsResource(true)
        {
        }

        /**
         * @brief Construct from existing Mix_Chunk (takes ownership)
         * @param existingChunk Existing Mix_Chunk pointer
         * @param takeOwnership If true, will free the chunk on destruction
         */
        explicit MixChunkRAII(Mix_Chunk* existingChunk, bool takeOwnership = true)
            : chunk(existingChunk)
            , filepath("")
            , ownsResource(takeOwnership)
        {
        }

        /**
         * @brief Destructor - automatically frees the Mix_Chunk
         */
        ~MixChunkRAII()
        {
            if (chunk && ownsResource)
            {
                Mix_FreeChunk(chunk);
                chunk = nullptr;
            }
        }

        // Delete copy constructor and copy assignment (non-copyable)
        MixChunkRAII(const MixChunkRAII&) = delete;
        MixChunkRAII& operator=(const MixChunkRAII&) = delete;

        /**
         * @brief Move constructor
         */
        MixChunkRAII(MixChunkRAII&& other) noexcept
            : chunk(other.chunk)
            , filepath(std::move(other.filepath))
            , ownsResource(other.ownsResource)
        {
            other.chunk = nullptr;
            other.ownsResource = false;
        }

        /**
         * @brief Move assignment operator
         */
        MixChunkRAII& operator=(MixChunkRAII&& other) noexcept
        {
            if (this != &other)
            {
                // Free current resource
                if (chunk && ownsResource)
                {
                    Mix_FreeChunk(chunk);
                }

                // Take ownership of other's resource
                chunk = other.chunk;
                filepath = std::move(other.filepath);
                ownsResource = other.ownsResource;

                // Invalidate other
                other.chunk = nullptr;
                other.ownsResource = false;
            }
            return *this;
        }

        /**
         * @brief Get the raw Mix_Chunk pointer
         * @return Mix_Chunk* or nullptr if not loaded
         */
        Mix_Chunk* get() const { return chunk; }

        /**
         * @brief Check if the chunk was loaded successfully
         * @return True if chunk is valid
         */
        bool isValid() const { return chunk != nullptr; }

        /**
         * @brief Get the file path (if loaded from file)
         * @return File path string
         */
        const std::string& getFilepath() const { return filepath; }

        /**
         * @brief Release ownership of the resource
         * @return The raw pointer (caller takes ownership)
         */
        Mix_Chunk* release()
        {
            ownsResource = false;
            return chunk;
        }

        /**
         * @brief Boolean conversion operator
         */
        explicit operator bool() const { return isValid(); }
    };

    /**
     * @brief RAII wrapper for Mix_Music
     *
     * Automatically loads and frees Mix_Music resources.
     */
    class MixMusicRAII
    {
    private:
        Mix_Music* music;
        std::string filepath;
        bool ownsResource;

    public:
        /**
         * @brief Construct from a file path
         * @param path Path to the music file
         */
        explicit MixMusicRAII(const char* path)
            : music(Mix_LoadMUS(path))
            , filepath(path ? path : "")
            , ownsResource(true)
        {
        }

        /**
         * @brief Construct from existing Mix_Music (takes ownership)
         * @param existingMusic Existing Mix_Music pointer
         * @param takeOwnership If true, will free the music on destruction
         */
        explicit MixMusicRAII(Mix_Music* existingMusic, bool takeOwnership = true)
            : music(existingMusic)
            , filepath("")
            , ownsResource(takeOwnership)
        {
        }

        /**
         * @brief Destructor - automatically frees the Mix_Music
         */
        ~MixMusicRAII()
        {
            if (music && ownsResource)
            {
                Mix_FreeMusic(music);
                music = nullptr;
            }
        }

        // Delete copy constructor and copy assignment (non-copyable)
        MixMusicRAII(const MixMusicRAII&) = delete;
        MixMusicRAII& operator=(const MixMusicRAII&) = delete;

        /**
         * @brief Move constructor
         */
        MixMusicRAII(MixMusicRAII&& other) noexcept
            : music(other.music)
            , filepath(std::move(other.filepath))
            , ownsResource(other.ownsResource)
        {
            other.music = nullptr;
            other.ownsResource = false;
        }

        /**
         * @brief Move assignment operator
         */
        MixMusicRAII& operator=(MixMusicRAII&& other) noexcept
        {
            if (this != &other)
            {
                // Free current resource
                if (music && ownsResource)
                {
                    Mix_FreeMusic(music);
                }

                // Take ownership of other's resource
                music = other.music;
                filepath = std::move(other.filepath);
                ownsResource = other.ownsResource;

                // Invalidate other
                other.music = nullptr;
                other.ownsResource = false;
            }
            return *this;
        }

        /**
         * @brief Get the raw Mix_Music pointer
         * @return Mix_Music* or nullptr if not loaded
         */
        Mix_Music* get() const { return music; }

        /**
         * @brief Check if the music was loaded successfully
         * @return True if music is valid
         */
        bool isValid() const { return music != nullptr; }

        /**
         * @brief Get the file path (if loaded from file)
         * @return File path string
         */
        const std::string& getFilepath() const { return filepath; }

        /**
         * @brief Release ownership of the resource
         * @return The raw pointer (caller takes ownership)
         */
        Mix_Music* release()
        {
            ownsResource = false;
            return music;
        }

        /**
         * @brief Boolean conversion operator
         */
        explicit operator bool() const { return isValid(); }
    };

    /**
     * @brief Smart pointer type aliases for convenience
     */
    using MixChunkPtr = std::unique_ptr<MixChunkRAII>;
    using MixMusicPtr = std::unique_ptr<MixMusicRAII>;

    /**
     * @brief Factory function to create a MixChunkPtr
     * @param path Path to audio file
     * @return Unique pointer to MixChunkRAII
     */
    inline MixChunkPtr makeMixChunk(const char* path)
    {
        return std::make_unique<MixChunkRAII>(path);
    }

    /**
     * @brief Factory function to create a MixMusicPtr
     * @param path Path to music file
     * @return Unique pointer to MixMusicRAII
     */
    inline MixMusicPtr makeMixMusic(const char* path)
    {
        return std::make_unique<MixMusicRAII>(path);
    }

} // namespace stevensSound
