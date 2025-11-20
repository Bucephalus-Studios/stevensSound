/**
 * @file test_pitch_shifting.cpp
 * @brief Unit tests for pitch shifting functionality
 */

#include <gtest/gtest.h>
#include <stevensSound.hpp>
#include <cmath>

/**
 * Test fixture for pitch shifting tests
 */
class PitchShiftingTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize SDL for tests
        initSound();

        // Create a simple test sound map
        std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
            {"test", {}}
        };
        stevensSound::init(sounds);
    }

    void TearDown() override
    {
        closeSound();
    }

    /**
     * Helper function to create a simple test Mix_Chunk with a sine wave
     */
    Mix_Chunk* createTestChunk(int durationMs = 100, int frequency = 440)
    {
        const int SAMPLE_RATE = 44100;
        const int CHANNELS = 2;
        const int BYTES_PER_SAMPLE = 2;

        // Calculate buffer size
        int numSamples = (SAMPLE_RATE * durationMs) / 1000;
        int bufferSize = numSamples * CHANNELS * BYTES_PER_SAMPLE;

        // Allocate buffer
        Uint8* buffer = new Uint8[bufferSize];
        Sint16* samples = reinterpret_cast<Sint16*>(buffer);

        // Generate a simple sine wave
        for (int i = 0; i < numSamples; i++)
        {
            float t = static_cast<float>(i) / SAMPLE_RATE;
            float value = std::sin(2.0f * M_PI * frequency * t);
            Sint16 sample = static_cast<Sint16>(value * 16384.0f); // 50% volume

            // Stereo - same value for both channels
            samples[i * 2] = sample;
            samples[i * 2 + 1] = sample;
        }

        // Create Mix_Chunk
        Mix_Chunk* chunk = new Mix_Chunk;
        chunk->allocated = 1;
        chunk->abuf = buffer;
        chunk->alen = bufferSize;
        chunk->volume = MIX_MAX_VOLUME;

        return chunk;
    }
};

/**
 * Test that applyPitchShift returns nullptr for invalid input
 */
TEST_F(PitchShiftingTest, NullInputReturnsNull)
{
    Mix_Chunk* result = stevensSound::applyPitchShift(nullptr, 1.0f);
    EXPECT_EQ(result, nullptr);
}

/**
 * Test that applyPitchShift returns nullptr for empty chunk
 */
TEST_F(PitchShiftingTest, EmptyChunkReturnsNull)
{
    Mix_Chunk emptyChunk;
    emptyChunk.abuf = nullptr;
    emptyChunk.alen = 0;
    emptyChunk.allocated = 0;
    emptyChunk.volume = MIX_MAX_VOLUME;

    Mix_Chunk* result = stevensSound::applyPitchShift(&emptyChunk, 1.0f);
    EXPECT_EQ(result, nullptr);
}

/**
 * Test that applyPitchShift returns a valid chunk for valid input
 */
TEST_F(PitchShiftingTest, ValidInputReturnsValidChunk)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    Mix_Chunk* shifted = stevensSound::applyPitchShift(original, 1.05f);

    ASSERT_NE(shifted, nullptr);
    EXPECT_NE(shifted->abuf, nullptr);
    EXPECT_GT(shifted->alen, 0);
    EXPECT_EQ(shifted->allocated, 1);

    // Clean up
    Mix_FreeChunk(shifted);
    Mix_FreeChunk(original);
}

/**
 * Test that original chunk is not modified
 */
TEST_F(PitchShiftingTest, OriginalChunkNotModified)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    // Store original properties
    Uint8* originalBuf = original->abuf;
    Uint32 originalLen = original->alen;

    Mix_Chunk* shifted = stevensSound::applyPitchShift(original, 1.05f);

    // Verify original is unchanged
    EXPECT_EQ(original->abuf, originalBuf);
    EXPECT_EQ(original->alen, originalLen);

    // Clean up
    if (shifted) Mix_FreeChunk(shifted);
    Mix_FreeChunk(original);
}

/**
 * Test pitch shifting with different multipliers
 */
TEST_F(PitchShiftingTest, DifferentPitchMultipliers)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    // Test various pitch multipliers
    std::vector<float> multipliers = {0.95f, 1.0f, 1.05f, 1.1f, 0.9f};

    for (float mult : multipliers)
    {
        Mix_Chunk* shifted = stevensSound::applyPitchShift(original, mult);

        ASSERT_NE(shifted, nullptr) << "Failed with multiplier " << mult;
        EXPECT_NE(shifted->abuf, nullptr);
        EXPECT_GT(shifted->alen, 0);

        Mix_FreeChunk(shifted);
    }

    Mix_FreeChunk(original);
}

/**
 * Test that shifted chunks can be stored persistently
 */
TEST_F(PitchShiftingTest, ShiftedChunkCanBeStoredPersistently)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    Mix_Chunk* shifted = stevensSound::applyPitchShift(original, 1.05f);
    ASSERT_NE(shifted, nullptr);

    // Store the shifted chunk
    stevensSound::storePersistentSound("test", "shifted_105", shifted);

    // Verify it's stored
    EXPECT_TRUE(stevensSound::isPersistentlyStored("test", "shifted_105"));

    // Clean up original (shifted is now owned by persistentChunks)
    Mix_FreeChunk(original);

    // Free the persistent sound
    stevensSound::freePersistentSound("test", "shifted_105");
}

/**
 * Test storePersistentSound with null chunk
 */
TEST_F(PitchShiftingTest, StorePersistentSoundRejectsNull)
{
    // Attempting to store null should fail gracefully
    stevensSound::storePersistentSound("test", "null_test", nullptr);

    // Should not be stored
    EXPECT_FALSE(stevensSound::isPersistentlyStored("test", "null_test"));
}

/**
 * Test multiple pitch variants can be created and stored
 */
TEST_F(PitchShiftingTest, MultipleVariantsCanBeCreated)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    // Create multiple variants
    std::vector<float> pitches = {0.95f, 1.0f, 1.05f};
    std::vector<std::string> names = {"variant_95", "variant_100", "variant_105"};

    for (size_t i = 0; i < pitches.size(); i++)
    {
        Mix_Chunk* variant = stevensSound::applyPitchShift(original, pitches[i]);
        ASSERT_NE(variant, nullptr);

        stevensSound::storePersistentSound("test", names[i], variant);
        EXPECT_TRUE(stevensSound::isPersistentlyStored("test", names[i]));
    }

    // Clean up
    Mix_FreeChunk(original);

    for (const auto& name : names)
    {
        stevensSound::freePersistentSound("test", name);
    }
}

/**
 * Test that volume is preserved during pitch shifting
 */
TEST_F(PitchShiftingTest, VolumeIsPreserved)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    Uint8 testVolume = 64; // 50% volume
    original->volume = testVolume;

    Mix_Chunk* shifted = stevensSound::applyPitchShift(original, 1.05f);
    ASSERT_NE(shifted, nullptr);

    EXPECT_EQ(shifted->volume, testVolume);

    // Clean up
    Mix_FreeChunk(shifted);
    Mix_FreeChunk(original);
}

/**
 * Test edge case: very small pitch shift
 */
TEST_F(PitchShiftingTest, VerySmallPitchShift)
{
    Mix_Chunk* original = createTestChunk(100, 440);
    ASSERT_NE(original, nullptr);

    // Very small pitch change (1% down)
    Mix_Chunk* shifted = stevensSound::applyPitchShift(original, 0.99f);
    ASSERT_NE(shifted, nullptr);

    EXPECT_NE(shifted->abuf, nullptr);
    EXPECT_GT(shifted->alen, 0);

    // Clean up
    Mix_FreeChunk(shifted);
    Mix_FreeChunk(original);
}
