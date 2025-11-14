/**
 * @file raii_example.cpp
 * @brief Example demonstrating RAII wrapper usage
 */

#include "../stevensSound.hpp"
#include <iostream>

int main()
{
    std::cout << "stevensSound RAII Wrappers Example\n";
    std::cout << "===================================\n\n";

    // Initialize SDL
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL/SDL_mixer\n";
        return 1;
    }

    std::cout << "Demonstrating RAII wrappers for automatic resource management...\n\n";

    // Example 1: Using MixChunkRAII directly
    std::cout << "1. Creating MixChunkRAII from file path:\n";
    {
        // Note: This would work with a real audio file
        stevensSound::MixChunkRAII chunk("path/to/sound.wav");

        if (chunk.isValid())
        {
            std::cout << "   Chunk loaded successfully!\n";
            std::cout << "   File: " << chunk.getFilepath() << "\n";
            // chunk.get() returns the raw Mix_Chunk* for SDL functions
        }
        else
        {
            std::cout << "   Failed to load chunk (expected with invalid path)\n";
        }
        // Chunk is automatically freed when it goes out of scope
        std::cout << "   Chunk will be automatically freed...\n";
    }
    std::cout << "   Chunk has been freed!\n\n";

    // Example 2: Using smart pointer with factory function
    std::cout << "2. Using makeMixChunk factory function:\n";
    {
        auto chunk = stevensSound::makeMixChunk("path/to/another_sound.wav");

        if (chunk && chunk->isValid())
        {
            std::cout << "   Chunk loaded via factory!\n";
        }
        else
        {
            std::cout << "   Failed to load (expected with invalid path)\n";
        }
        // Automatic cleanup via unique_ptr
    }
    std::cout << "   Smart pointer cleaned up automatically!\n\n";

    // Example 3: Move semantics
    std::cout << "3. Demonstrating move semantics:\n";
    {
        stevensSound::MixChunkRAII chunk1("path/to/sound1.wav");
        std::cout << "   Created chunk1\n";

        // Move chunk1 into chunk2
        stevensSound::MixChunkRAII chunk2(std::move(chunk1));
        std::cout << "   Moved chunk1 -> chunk2\n";
        std::cout << "   chunk1 is now invalid: " << (chunk1.isValid() ? "false" : "true") << "\n";
        std::cout << "   chunk2 owns the resource now\n";
    }
    std::cout << "   Both wrappers out of scope, resource freed once\n\n";

    // Example 4: MixMusicRAII
    std::cout << "4. Using MixMusicRAII for music:\n";
    {
        stevensSound::MixMusicRAII music("path/to/music.mp3");

        if (music.isValid())
        {
            std::cout << "   Music loaded successfully!\n";
            // You could play it with Mix_PlayMusic(music.get(), -1);
        }
        else
        {
            std::cout << "   Failed to load music (expected)\n";
        }
    }
    std::cout << "   Music automatically freed!\n\n";

    // Example 5: Release ownership
    std::cout << "5. Releasing ownership:\n";
    {
        stevensSound::MixChunkRAII chunk("path/to/sound.wav");
        Mix_Chunk* rawChunk = chunk.release();
        std::cout << "   Ownership released, manual cleanup required\n";

        // Now you must manually free it
        if (rawChunk)
        {
            Mix_FreeChunk(rawChunk);
            std::cout << "   Manually freed the chunk\n";
        }
    }

    std::cout << "\nKey benefits of RAII wrappers:\n";
    std::cout << "  - Automatic resource cleanup (no memory leaks)\n";
    std::cout << "  - Exception safe\n";
    std::cout << "  - Move semantics for efficient transfer\n";
    std::cout << "  - Clear ownership semantics\n";
    std::cout << "  - No need for manual Mix_FreeChunk/Mix_FreeMusic calls\n";

    // Clean up
    std::cout << "\nCleaning up SDL...\n";
    closeSound();

    std::cout << "Example completed!\n";
    return 0;
}
