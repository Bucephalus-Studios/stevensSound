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
    }

    void TearDown() override
    {
        closeSound();
    }
};

TEST_F(SoundPlaybackTest, PlayInvalidSound)
{
    // Playing a sound that doesn't exist should be a safe no-op (logged via spdlog,
    // not something this test can observe) rather than a crash.
    EXPECT_FALSE(stevensSound::soundsContains("sfx", "nonexistent"));
    stevensSound::playSound("sfx", "nonexistent");
}

TEST_F(SoundPlaybackTest, CreateSoundPlaylist)
{
    std::vector<std::string> categories = {"sfx"};
    std::vector<std::string> trackOrder = {};

    stevensSound::SoundPlaylist playlist =
        stevensSound::createSoundPlaylist("test_playlist", "sfx", categories, trackOrder, false);

    EXPECT_EQ(playlist.name, "test_playlist");
    EXPECT_EQ(playlist.controllerId, "sfx");
}

TEST_F(SoundPlaybackTest, SwitchToNonExistentPlaylist)
{
    // Switching to a playlist that doesn't exist should be a safe no-op (logged via
    // spdlog, not something this test can observe) rather than a crash or a queued command.
    ASSERT_FALSE(stevensSound::playlists.contains("nonexistent_playlist"));
    stevensSound::switchMusicPlaylist("nonexistent_playlist");
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
