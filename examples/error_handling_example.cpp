/**
 * @file error_handling_example.cpp
 * @brief Example demonstrating error handling features
 */

#include "../stevensSound.hpp"
#include <iostream>

int main()
{
    std::cout << "stevensSound Error Handling Example\n";
    std::cout << "====================================\n\n";

    // Enable error logging to stdout
    stevensSound::ErrorHandler::setLogging(true);
    std::cout << "Error logging enabled\n\n";

    // Set up a custom error handler
    stevensSound::ErrorHandler::setErrorHandler([](const stevensSound::ErrorInfo& error) {
        std::cout << "\n[Custom Handler] Caught error:\n";
        std::cout << "  Level: ";
        switch(error.level)
        {
            case stevensSound::ErrorLevel::INFO: std::cout << "INFO"; break;
            case stevensSound::ErrorLevel::WARNING: std::cout << "WARNING"; break;
            case stevensSound::ErrorLevel::ERROR: std::cout << "ERROR"; break;
            case stevensSound::ErrorLevel::CRITICAL: std::cout << "CRITICAL"; break;
        }
        std::cout << "\n  Message: " << error.message << "\n";
        std::cout << "  Function: " << error.function << "\n";
        std::cout << "  Timestamp: " << error.timestamp << "\n";
    });

    // Initialize SDL
    if (!initSound())
    {
        std::cerr << "\nFailed to initialize! This is expected if SDL is not available.\n";
        return 1;
    }

    // Initialize library
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}},
        {"music", {}}
    };

    stevensSound::init(sounds);

    // Test different error scenarios
    std::cout << "\n--- Testing Error Scenarios ---\n\n";

    // Scenario 1: Play non-existent sound
    std::cout << "1. Attempting to play non-existent sound...\n";
    stevensSound::playSound("sfx", "does_not_exist");

    // Scenario 2: Switch to non-existent playlist
    std::cout << "\n2. Attempting to switch to non-existent playlist...\n";
    stevensSound::switchMusicPlaylist("imaginary_playlist");

    // Check final error state
    std::cout << "\n--- Final Error State ---\n";
    if (stevensSound::ErrorHandler::hasError())
    {
        std::cout << "Last error message: " << stevensSound::ErrorHandler::getLastErrorMessage() << "\n";
    }

    // Clean up
    std::cout << "\nCleaning up...\n";
    closeSound();

    std::cout << "\nExample completed!\n";
    return 0;
}
