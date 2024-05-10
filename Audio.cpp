// NECHAT JAK JE
#include <iostream>

#include "Audio.hpp"

Audio::Audio()
{
    // Create the sound engine
    engine = irrklang::createIrrKlangDevice();

    // Add sound sources for different audio effects
    irrklang::ISoundSource* sound_breake_glass = engine->addSoundSourceFromFile("resources/sounds/glass.mp3");
    sounds.insert({ "sound_glass", sound_breake_glass });

    irrklang::ISoundSource* sound_shoot = engine->addSoundSourceFromFile("resources/sounds/gun.mp3");
    sounds.insert({ "sound_shoot", sound_shoot });

    irrklang::ISoundSource* sound_land = engine->addSoundSourceFromFile("resources/sounds/land.mp3");
    sounds.insert({ "sound_land", sound_land });

    irrklang::ISoundSource* sound_jump = engine->addSoundSourceFromFile("resources/sounds/jump.mp3");
    sounds.insert({ "sound_jump", sound_jump });

    irrklang::ISoundSource* sound_first_step = engine->addSoundSourceFromFile("resources/sounds/first_step.mp3");
    sounds.insert({ "sound_first_step", sound_first_step });

    irrklang::ISoundSource* sound_second_step = engine->addSoundSourceFromFile("resources/sounds/second_step.mp3");
    sounds.insert({ "sound_second_step", sound_second_step });
}

// Function to play a shot sound
void Audio::PlayShot(std::string sound_name)
{
    engine->play2D(sounds[sound_name]);
}

// Function to play a land sound
void Audio::PlayLand()
{
    engine->play2D(sounds["sound_land"]);
}

// Function to play a jump sound
void Audio::PlayJump()
{
    engine->play2D(sounds["sound_jump"]);
}

// Function to play walking sounds alternately
void Audio::PlayWalk()
{
    engine->play2D(sounds[walkSwitch ? "sound_first_step" : "sound_second_step"]);
    walkSwitch = !walkSwitch;
}

// Destructor for the Audio class
Audio::~Audio()
{
    // Check if the sound engine exists and release it
    if (engine) {
        engine->drop();
    }
}