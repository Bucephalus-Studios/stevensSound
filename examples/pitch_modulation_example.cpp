/**
 * @file pitch_modulation_example.cpp
 * @brief Example demonstrating pitch modulation and audio effects
 */

#include "../stevensSound.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    std::cout << "stevensSound Pitch Modulation Example\n";
    std::cout << "======================================\n\n";

    // Initialize SDL
    if (!initSound())
    {
        std::cerr << "Failed to initialize SDL/SDL_mixer\n";
        return 1;
    }

    // Initialize library
    std::unordered_map<std::string, std::unordered_map<std::string, const char*>> sounds = {
        {"sfx", {
            // In a real application, provide actual sound files
            // {"button_click", "assets/button.wav"},
            // {"notification", "assets/notify.wav"}
        }}
    };

    stevensSound::init(sounds);

    std::cout << "Audio Effects and Anti-Fatigue Features\n";
    std::cout << "----------------------------------------\n\n";

    // Example 1: Set up anti-fatigue for UI sounds
    std::cout << "1. Setting up anti-fatigue for button clicks:\n";
    std::cout << "   This prevents sound fatigue from repetitive UI sounds\n";
    std::cout << "   by adding slight pitch variation on each play.\n\n";

    // Enable pitch randomization (10% variation)
    stevensSound::setupAntiFatigueSound("sfx", "button_click", 0.1f);
    std::cout << "   - Enabled pitch randomization (±10%)\n";

    // Example 2: Manual effects configuration
    std::cout << "\n2. Configuring custom audio effects:\n";
    stevensSound::AudioEffects customEffects;
    customEffects.volumeModulation = 0.8f;  // Slightly quieter
    customEffects.panPosition = -0.3f;      // Pan slightly to left
    customEffects.randomizePitch = true;
    customEffects.randomRange = 0.15f;      // 15% variation

    stevensSound::AudioEffectsManager::setEffects("sfx", "notification", customEffects);
    std::cout << "   - Volume: 80%\n";
    std::cout << "   - Pan: 30% left\n";
    std::cout << "   - Pitch randomization: ±15%\n";

    // Example 3: Panning effects
    std::cout << "\n3. Setting pan position for spatial audio:\n";
    std::cout << "   Pan values: -1.0 (full left) to 1.0 (full right)\n\n";

    // Pan to left
    stevensSound::AudioEffectsManager::setPanPosition("sfx", "sound_left", -1.0f);
    std::cout << "   - sound_left: Full left\n";

    // Pan to center
    stevensSound::AudioEffectsManager::setPanPosition("sfx", "sound_center", 0.0f);
    std::cout << "   - sound_center: Center\n";

    // Pan to right
    stevensSound::AudioEffectsManager::setPanPosition("sfx", "sound_right", 1.0f);
    std::cout << "   - sound_right: Full right\n";

    // Example 4: Getting current effects
    std::cout << "\n4. Retrieving current effects:\n";
    auto effects = stevensSound::AudioEffectsManager::getEffects("sfx", "notification");
    std::cout << "   notification effects:\n";
    std::cout << "     - Volume modulation: " << effects.volumeModulation << "\n";
    std::cout << "     - Pan position: " << effects.panPosition << "\n";
    std::cout << "     - Randomize pitch: " << (effects.randomizePitch ? "Yes" : "No") << "\n";
    std::cout << "     - Random range: " << effects.randomRange << "\n";

    // Example 5: Demonstrate randomization
    std::cout << "\n5. Demonstrating pitch randomization:\n";
    std::cout << "   Generating 5 random pitch variations:\n";
    for (int i = 0; i < 5; i++)
    {
        float variation = stevensSound::AudioEffectsManager::getRandomPitchVariation(0.1f);
        std::cout << "     Variation " << (i+1) << ": " << (variation >= 0 ? "+" : "")
                  << variation << " (" << (variation * 100) << "%)\n";
    }

    std::cout << "\n\nIMPORTANT NOTES:\n";
    std::cout << "================\n";
    std::cout << "1. SDL_mixer does not support true pitch shifting without tempo changes.\n";
    std::cout << "2. This implementation provides volume and panning effects.\n";
    std::cout << "3. For true pitch shifting, consider integrating:\n";
    std::cout << "   - SoLoud library (https://solhsa.com/soloud/)\n";
    std::cout << "   - libsoundtouch (https://www.surina.net/soundtouch/)\n";
    std::cout << "4. The randomization prevents audio fatigue by varying playback slightly.\n";

    std::cout << "\n\nUsage Pattern for UI Sounds:\n";
    std::cout << "----------------------------\n";
    std::cout << "// Set up once during initialization\n";
    std::cout << "setupAntiFatigueSound(\"sfx\", \"button_click\", 0.1f);\n";
    std::cout << "setupAntiFatigueSound(\"sfx\", \"hover_sound\", 0.08f);\n\n";
    std::cout << "// Then just play normally - effects are applied automatically\n";
    std::cout << "playSound(\"sfx\", \"button_click\");\n";

    // Clean up
    std::cout << "\nCleaning up...\n";
    stevensSound::AudioEffectsManager::clearAllEffects();
    closeSound();

    std::cout << "Example completed!\n";
    return 0;
}
