/**
 * @file test_sound_playback.cpp
 * @brief Tests for sound playback functionality
 */

#include <gtest/gtest.h>
#include "../stevensSound.hpp"

class SoundPlaybackTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        if (!initSound())
        {
            GTEST_SKIP() << "Failed to initialize SDL/SDL_mixer";
        }

        std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
            {"sfx", {}},
            {"music", {}}
        };

        stevensSound::init(sounds);
        stevensSound::ErrorHandler::clearError();
    }

    void TearDown() override
    {
        closeSound();
    }
};

TEST_F(SoundPlaybackTest, PlayInvalidSound)
{
    // Try to play a sound that doesn't exist
    stevensSound::playSound("sfx", "nonexistent");

    EXPECT_TRUE(stevensSound::ErrorHandler::hasError());
    auto error = stevensSound::ErrorHandler::getLastError();
    EXPECT_EQ(error.level, stevensSound::ErrorLevel::ERROR);
}

TEST_F(SoundPlaybackTest, StorePersistentSoundError)
{
    // Try to store a sound that doesn't exist
    stevensSound::storePersistentSound("sfx", "nonexistent");

    EXPECT_TRUE(stevensSound::ErrorHandler::hasError());
    auto error = stevensSound::ErrorHandler::getLastError();
    EXPECT_EQ(error.level, stevensSound::ErrorLevel::ERROR);
}

TEST_F(SoundPlaybackTest, FreePersistentSoundError)
{
    // Try to free a sound that isn't persistently stored
    stevensSound::freePersistentSound("sfx", "nonexistent");

    EXPECT_TRUE(stevensSound::ErrorHandler::hasError());
    auto error = stevensSound::ErrorHandler::getLastError();
    EXPECT_EQ(error.level, stevensSound::ErrorLevel::ERROR);
}

TEST_F(SoundPlaybackTest, CreatePlaylist)
{
    stevensSound::ErrorHandler::clearError();

    std::vector<std::string> categories = {"sfx"};
    std::vector<std::string> trackOrder = {};

    stevensSound::createPlaylist("test_playlist", "sfx", categories, trackOrder, false);

    // Should complete without error
    EXPECT_FALSE(stevensSound::ErrorHandler::hasError());
}

TEST_F(SoundPlaybackTest, SwitchToNonExistentPlaylist)
{
    stevensSound::ErrorHandler::clearError();

    // Try to switch to a playlist that doesn't exist
    stevensSound::switchMusicPlaylist("nonexistent_playlist");

    EXPECT_TRUE(stevensSound::ErrorHandler::hasError());
    auto error = stevensSound::ErrorHandler::getLastError();
    EXPECT_EQ(error.level, stevensSound::ErrorLevel::ERROR);
}

TEST_F(SoundPlaybackTest, VolumeController)
{
    // Get the default controller
    EXPECT_TRUE(stevensSound::soundControllers.contains("default"));
    EXPECT_TRUE(stevensSound::soundControllers.contains("sfx"));
    EXPECT_TRUE(stevensSound::soundControllers.contains("music"));

    // Check default volume
    EXPECT_FLOAT_EQ(stevensSound::soundControllers["default"].volume, 1.0f);
    EXPECT_FLOAT_EQ(stevensSound::soundControllers["sfx"].volume, 1.0f);
    EXPECT_FLOAT_EQ(stevensSound::soundControllers["music"].volume, 1.0f);

    // Modify volume
    stevensSound::soundControllers["sfx"].volume = 0.5f;
    EXPECT_FLOAT_EQ(stevensSound::soundControllers["sfx"].volume, 0.5f);
}
