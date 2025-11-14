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

    // Initialize SDL and SDL_mixer
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL/SDL_mixer\n";
        std::cerr << "Error: " << stevensSound::ErrorHandler::getLastErrorMessage() << "\n";
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
        std::cerr << "Error: " << stevensSound::ErrorHandler::getLastErrorMessage() << "\n";
        closeSound();
        return 1;
    }

    std::cout << "Library initialized successfully!\n\n";

    // Demonstrate error handling
    std::cout << "Testing error handling...\n";
    stevensSound::ErrorHandler::clearError();

    // Try to play a sound that doesn't exist
    stevensSound::playSound("sfx", "nonexistent_sound");

    if (stevensSound::ErrorHandler::hasError())
    {
        auto error = stevensSound::ErrorHandler::getLastError();
        std::cout << "Caught expected error:\n";
        std::cout << "  " << error.toString() << "\n\n";
    }

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
