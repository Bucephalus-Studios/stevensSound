/**
 * @file playlist_example.cpp
 * @brief Example demonstrating playlist functionality
 */

#include "../stevensSound.hpp"
#include <iostream>

int main()
{
    std::cout << "stevensSound Playlist Example\n";
    std::cout << "=============================\n\n";

    // Initialize SDL
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL/SDL_mixer\n";
        return 1;
    }

    // Initialize library with music category
    // In a real application, you would provide paths to actual music files
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"music", {
            // Example: {"song1", "path/to/song1.wav"},
            //          {"song2", "path/to/song2.wav"}
        }}
    };

    stevensSound::init(sounds);

    std::cout << "Creating a music playlist...\n";

    // Create a playlist
    std::vector<std::string> categories = {"music"};
    std::vector<std::string> trackOrder = {}; // Empty for now

    stevensSound::createPlaylist(
        "my_playlist",      // Playlist name
        "music",            // Controller ID
        categories,         // Sound categories to use
        trackOrder,         // Track order
        false               // Shuffle fill
    );

    std::cout << "Playlist 'my_playlist' created successfully!\n\n";

    // Demonstrate playlist existence check
    if (stevensSound::playlists.contains("my_playlist"))
    {
        std::cout << "Playlist found in library!\n";
        std::cout << "Playlist status: " << stevensSound::playlists["my_playlist"].status << "\n";
        std::cout << "Number of tracks: " << stevensSound::playlists["my_playlist"].sounds.size() << "\n";
    }

    // Volume control for music
    std::cout << "\nMusic volume control:\n";
    std::cout << "  Current volume: " << stevensSound::soundControllers["music"].volume << "\n";
    std::cout << "  Setting volume to 0.7...\n";
    stevensSound::soundControllers["music"].volume = 0.7f;
    std::cout << "  New volume: " << stevensSound::soundControllers["music"].volume << "\n";

    // Note: To actually play the playlist, you would use:
    // std::thread musicThread(stevensSound::playMusicPlaylist,
    //                        std::ref(stevensSound::playlists["my_playlist"]),
    //                        "loop");
    // musicThread.detach();

    std::cout << "\nNote: Actual playback requires valid audio files.\n";
    std::cout << "This example demonstrates playlist creation and management.\n";

    // Clean up
    std::cout << "\nCleaning up...\n";
    closeSound();

    std::cout << "Example completed!\n";
    return 0;
}
