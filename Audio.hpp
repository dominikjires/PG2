#pragma once
#include <glm/glm.hpp>
#include <irrKlang/irrKlang.h>
#include <map>

class Audio
{
public:
	Audio();
	void PlayShot(std::string sound_name);
	void PlayLand();
	void PlayJump();
	void PlayWalk();
	~Audio();
private:
	irrklang::ISoundEngine* engine = nullptr;
	std::map<std::string, irrklang::ISoundSource*> sounds;
	irrklang::ISound* fall = nullptr;
	irrklang::ISound* land = nullptr;

	bool walkSwitch = false;
};
