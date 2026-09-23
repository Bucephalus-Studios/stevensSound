/**
 * @file basic_example.cpp
 * @brief Basic example demonstrating stevensSound library usage
 */

#include "../stevensSound.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    std::cout << "stevensSound Basic Example\n";
    std::cout << "==========================\n\n";

    // Initialize SDL and SDL_mixer (failure details are logged via spdlog)
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL/SDL_mixer\n";
        return 1;
    }

    // Print SDL version information
    std::cout << stevensSound::getSDLVersionInfo() << "\n";

    // Initialize the stevensSound library
    // In a real application, you would provide paths to actual sound files
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}},
        {"music", {}}
    };

    if (!stevensSound::init(sounds))
    {
        std::cerr << "Failed to initialize stevensSound library\n";
        closeSound();
        return 1;
    }

    std::cout << "Library initialized successfully!\n\n";

    // Playing a sound that doesn't exist is a safe no-op -- stevensSound logs the
    // failure via spdlog rather than surfacing it through a return value here.
    std::cout << "Testing playback of a non-existent sound (see spdlog output)...\n";
    stevensSound::playSound("sfx", "nonexistent_sound");

    // Demonstrate volume control
    std::cout << "Volume control example:\n";
    std::cout << "  SFX volume: " << stevensSound::soundControllers["sfx"].volume << "\n";
    std::cout << "  Setting SFX volume to 0.5...\n";
    stevensSound::soundControllers["sfx"].volume = 0.5f;
    std::cout << "  New SFX volume: " << stevensSound::soundControllers["sfx"].volume << "\n\n";

    // Clean up
    std::cout << "Cleaning up...\n";
    closeSound();

    std::cout << "Example completed successfully!\n";
    return 0;
}
