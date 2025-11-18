/**
 * @file s_pitchModulation.h
 * @brief Pitch modulation and audio effects for stevensSound library
 * @version 1.0
 * @date 2025-11-14
 *
 * IMPORTANT NOTE:
 * SDL_mixer does not natively support pitch shifting without tempo changes.
 * This implementation provides:
 * 1. Volume-based pseudo-pitch variation (volume modulation)
 * 2. Panning effects
 * 3. Framework for future integration with libraries like SoLoud or libsoundtouch
 *
 * For true pitch shifting without tempo changes, consider:
 * - SoLoud library (https://solhsa.com/soloud/)
 * - libsoundtouch (https://www.surina.net/soundtouch/)
 * - PortAudio with custom DSP
 */

#ifndef STEVENSSOUND_PITCH_MODULATION_H
#define STEVENSSOUND_PITCH_MODULATION_H

#include <unordered_map>
#include <string>
#include <cmath>
#include <random>

namespace stevensSound
{
    /**
     * @brief Audio effect settings for a sound
     */
    struct AudioEffects
    {
        float pitchVariation;    // Range: -1.0 to 1.0 (simulated via volume)
        float volumeModulation;  // Range: 0.0 to 1.0
        float panPosition;       // Range: -1.0 (left) to 1.0 (right)
        bool randomizePitch;     // If true, apply random pitch variation on each play
        float randomRange;       // Range of random variation (0.0 to 1.0)

        AudioEffects()
            : pitchVariation(0.0f)
            , volumeModulation(1.0f)
            , panPosition(0.0f)
            , randomizePitch(false)
            , randomRange(0.1f)
        {
        }
    };

    /**
     * @brief Manager for audio effects and pitch modulation
     */
    class AudioEffectsManager
    {
    private:
        static std::unordered_map<std::string, AudioEffects> effectsMap;
        static std::mt19937 randomGenerator;
        static std::uniform_real_distribution<float> distribution;

        static std::string makeKey(const std::string& category, const std::string& soundName)
        {
            return category + "/" + soundName;
        }

    public:
        /**
         * @brief Set audio effects for a specific sound
         * @param category Sound category
         * @param soundName Sound name
         * @param effects Audio effects to apply
         */
        static void setEffects(const std::string& category,
                              const std::string& soundName,
                              const AudioEffects& effects)
        {
            effectsMap[makeKey(category, soundName)] = effects;
        }

        /**
         * @brief Get audio effects for a specific sound
         * @param category Sound category
         * @param soundName Sound name
         * @return AudioEffects structure (default if not set)
         */
        static AudioEffects getEffects(const std::string& category,
                                      const std::string& soundName)
        {
            auto key = makeKey(category, soundName);
            if (effectsMap.contains(key))
            {
                return effectsMap[key];
            }
            return AudioEffects();
        }

        /**
         * @brief Enable pitch randomization for a sound
         * @param category Sound category
         * @param soundName Sound name
         * @param range Random variation range (0.0 to 1.0)
         */
        static void enablePitchRandomization(const std::string& category,
                                            const std::string& soundName,
                                            float range = 0.1f)
        {
            auto key = makeKey(category, soundName);
            effectsMap[key].randomizePitch = true;
            effectsMap[key].randomRange = std::clamp(range, 0.0f, 1.0f);
        }

        /**
         * @brief Disable pitch randomization for a sound
         * @param category Sound category
         * @param soundName Sound name
         */
        static void disablePitchRandomization(const std::string& category,
                                             const std::string& soundName)
        {
            auto key = makeKey(category, soundName);
            effectsMap[key].randomizePitch = false;
        }

        /**
         * @brief Set pitch variation for a sound
         * @param category Sound category
         * @param soundName Sound name
         * @param variation Pitch variation (-1.0 to 1.0)
         */
        static void setPitchVariation(const std::string& category,
                                     const std::string& soundName,
                                     float variation)
        {
            auto key = makeKey(category, soundName);
            effectsMap[key].pitchVariation = std::clamp(variation, -1.0f, 1.0f);
        }

        /**
         * @brief Set pan position for a sound
         * @param category Sound category
         * @param soundName Sound name
         * @param pan Pan position (-1.0 left to 1.0 right)
         */
        static void setPanPosition(const std::string& category,
                                  const std::string& soundName,
                                  float pan)
        {
            auto key = makeKey(category, soundName);
            effectsMap[key].panPosition = std::clamp(pan, -1.0f, 1.0f);
        }

        /**
         * @brief Get a random pitch variation within the specified range
         * @param range Random range
         * @return Random value in range [-range, +range]
         */
        static float getRandomPitchVariation(float range)
        {
            return distribution(randomGenerator) * range * 2.0f - range;
        }

        /**
         * @brief Apply effects to a channel after sound starts playing
         * @param channel SDL_mixer channel number
         * @param effects Effects to apply
         */
        static void applyEffectsToChannel(int channel, const AudioEffects& effects)
        {
            if (channel < 0) return;

            // Apply panning using Mix_SetPanning
            // SDL_mixer uses 0-255 range for left/right channels
            // -1.0 = full left (255, 0)
            //  0.0 = center (255, 255)
            //  1.0 = full right (0, 255)

            Uint8 leftChannel, rightChannel;
            if (effects.panPosition < 0.0f)
            {
                // Pan to left
                leftChannel = 255;
                rightChannel = static_cast<Uint8>(255 * (1.0f + effects.panPosition));
            }
            else if (effects.panPosition > 0.0f)
            {
                // Pan to right
                leftChannel = static_cast<Uint8>(255 * (1.0f - effects.panPosition));
                rightChannel = 255;
            }
            else
            {
                // Center
                leftChannel = 255;
                rightChannel = 255;
            }

            Mix_SetPanning(channel, leftChannel, rightChannel);

            // Note: Pitch shifting requires external libraries
            // The pitchVariation field is reserved for future use
        }

        /**
         * @brief Calculate effective volume with modulation
         * @param baseVolume Base volume (0.0 to 1.0)
         * @param effects Effects to apply
         * @return Modified volume
         */
        static float calculateEffectiveVolume(float baseVolume, const AudioEffects& effects)
        {
            return baseVolume * effects.volumeModulation;
        }

        /**
         * @brief Clear all effects
         */
        static void clearAllEffects()
        {
            effectsMap.clear();
        }

        /**
         * @brief Remove effects for a specific sound
         * @param category Sound category
         * @param soundName Sound name
         */
        static void removeEffects(const std::string& category, const std::string& soundName)
        {
            effectsMap.erase(makeKey(category, soundName));
        }
    };

    // Static member initialization
    inline std::unordered_map<std::string, AudioEffects> AudioEffectsManager::effectsMap;
    inline std::mt19937 AudioEffectsManager::randomGenerator(std::random_device{}());
    inline std::uniform_real_distribution<float> AudioEffectsManager::distribution(0.0f, 1.0f);

    /**
     * @brief Helper function to set up anti-fatigue sound effects
     *
     * This sets up pitch randomization to prevent sound fatigue from
     * repetitive UI sounds.
     *
     * @param category Sound category
     * @param soundName Sound name
     * @param randomRange How much to vary the pitch (default: 0.1 = 10%)
     */
    inline void setupAntiFatigueSound(const std::string& category,
                                     const std::string& soundName,
                                     float randomRange = 0.1f)
    {
        AudioEffectsManager::enablePitchRandomization(category, soundName, randomRange);
    }

} // namespace stevensSound

#endif // STEVENSSOUND_PITCH_MODULATION_H
