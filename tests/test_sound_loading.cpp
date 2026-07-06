/**
 * @file test_sound_loading.cpp
 * @brief Tests for sound loading functionality
 */

#include <gtest/gtest.h>
#include "../stevensSound.hpp"

class SoundLoadingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize SDL for testing
        if (!initSound())
        {
            GTEST_SKIP() << "Failed to initialize SDL/SDL_mixer";
        }

        stevensSound::ErrorHandler::clearError();
    }

    void TearDown() override
    {
        closeSound();
    }
};

TEST_F(SoundLoadingTest, InitializeLibrary)
{
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}}
    };

    bool success = stevensSound::init(sounds);
    EXPECT_TRUE(success);
}

TEST_F(SoundLoadingTest, GetSDLVersionInfo)
{
    std::string versionInfo = stevensSound::getSDLVersionInfo();

    EXPECT_FALSE(versionInfo.empty());
    EXPECT_NE(versionInfo.find("SDL Version:"), std::string::npos);
    EXPECT_NE(versionInfo.find("SDL Mixer Version:"), std::string::npos);
}

TEST_F(SoundLoadingTest, SoundsContains)
{
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}}
    };

    stevensSound::init(sounds);

    // Should not contain non-existent sound
    EXPECT_FALSE(stevensSound::soundsContains("sfx", "nonexistent"));
    EXPECT_FALSE(stevensSound::soundsContains("music", "nonexistent"));
}

TEST_F(SoundLoadingTest, ErrorOnInvalidSound)
{
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {}}
    };

    stevensSound::init(sounds);
    stevensSound::ErrorHandler::clearError();

    // Try to play non-existent sound
    stevensSound::playSound("sfx", "nonexistent");

    // Should have set an error
    EXPECT_TRUE(stevensSound::ErrorHandler::hasError());
    auto error = stevensSound::ErrorHandler::getLastError();
    EXPECT_EQ(error.level, stevensSound::ErrorLevel::ERROR);
}
