#pragma once
#include <map>

#include <glm/glm.hpp>
#include <irrKlang/irrKlang.h>

class AudioSlave
{
public:
	AudioSlave();

	// All 3D sound
	void UpdateListenerPosition(glm::vec3 position, glm::vec3 front, glm::vec3 world_up);
	// Jukebox
	void PlayMusic3D();
	void UpdateMusicPosition(glm::vec3 position);
	void UpdateMusicVolume(float amount);
	// Sound effects
	void Play2DOneShot(std::string sound_name);
	void Play3DOneShot(std::string sound_name, glm::vec3 position);
	void PlayWalk(); // Switches between two walk sounds automatically
	// Jetpack	
	void InitJetpack();
	void UpdateJetpackVolume(bool unmute);

	~AudioSlave();
private:
	irrklang::ISoundEngine* engine = nullptr;

	std::map<std::string, irrklang::ISoundSource*> sounds; // Map of sound effects

	irrklang::ISound* music = nullptr; // Jukebox music
	irrklang::ISound* jetpack = nullptr;

	bool walkSwitch = false;
};
