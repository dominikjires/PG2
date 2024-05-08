#include <iostream>

#include "AudioSlave.hpp"

#define MUSIC_VOLUME 0.3f
#define JETPACK_VOLUME 0.15f

AudioSlave::AudioSlave()
{
	engine = irrklang::createIrrKlangDevice();
	if (!engine) {
		std::cerr << "[!] IrrKlang device creation FAILED.\n";
	}

	// Init SFXs
	irrklang::ISoundSource* snd_step1 = engine->addSoundSourceFromFile("resources/sfx/sand2.ogg");
	snd_step1->setDefaultVolume(0.25f);
	sounds.insert({ "snd_step1", snd_step1 });
	
	irrklang::ISoundSource* snd_step2 = engine->addSoundSourceFromFile("resources/sfx/sand3.ogg");
	snd_step2->setDefaultVolume(0.25f);
	sounds.insert({ "snd_step2", snd_step2 });
	
	irrklang::ISoundSource* snd_glass = engine->addSoundSourceFromFile("resources/sfx/glass.mp3");
	sounds.insert({ "snd_glass", snd_glass });
	
	irrklang::ISoundSource* snd_shoot = engine->addSoundSourceFromFile("resources/sfx/bow.mp3");
	snd_shoot->setDefaultVolume(0.45f);
	sounds.insert({ "snd_shoot", snd_shoot });

	irrklang::ISoundSource* snd_hit = engine->addSoundSourceFromFile("resources/sfx/hit.mp3");
	sounds.insert({ "snd_hit", snd_hit });

	InitJetpack();
}

void AudioSlave::UpdateListenerPosition(glm::vec3 position, glm::vec3 front, glm::vec3 world_up)
{
	// position of the listener
	irrklang::vec3df _position(position.x, position.y, position.z);
	// the direction the listener looks into
	irrklang::vec3df _look_direction(-front.x, front.y, -front.z);
	// only relevant for doppler effects
	irrklang::vec3df _vel_per_second(0, 0, 0);
	// where 'up' is in your 3D scene
	irrklang::vec3df _up_vector(world_up.x, world_up.y, world_up.z);
	
	engine->setListenerPosition(_position, _look_direction, _vel_per_second, _up_vector);
}

void AudioSlave::PlayMusic3D()
{
	music = engine->play3D("resources/music/despacito.mp3", irrklang::vec3df(0, 0, 0), true, true);
	if (music) {
		music->setMinDistance(1.0f);
		music->setIsPaused(false);
		music->setVolume(MUSIC_VOLUME);
	}
}

void AudioSlave::UpdateMusicPosition(glm::vec3 position)
{
	irrklang::vec3df _position(position.x, position.y, position.z);
	music->setPosition(_position);
}

void AudioSlave::UpdateMusicVolume(float amount)
{
	music->setVolume(amount * MUSIC_VOLUME);
}

void AudioSlave::Play2DOneShot(std::string sound_name)
{
	engine->play2D(sounds[sound_name]);
}

void AudioSlave::Play3DOneShot(std::string sound_name, glm::vec3 position)
{
	irrklang::vec3df _position(position.x, position.y, position.z);
	engine->play3D(sounds[sound_name], _position);
}

void AudioSlave::PlayWalk()
{
	engine->play2D(sounds[walkSwitch ? "snd_step1" : "snd_step2"]);
	walkSwitch = !walkSwitch;
}

void AudioSlave::InitJetpack()
{
	jetpack = engine->play2D("resources/sfx/jetpack.mp3", true, true);
	if (jetpack) {
		jetpack->setVolume(JETPACK_VOLUME);		
	}
}

void AudioSlave::UpdateJetpackVolume(bool unmute)
{
	jetpack->setIsPaused(!unmute);
}

AudioSlave::~AudioSlave()
{
	if (music) {
		music->stop();
		music->drop();
	}
	if (jetpack) {
		jetpack->stop();
		jetpack->drop();
	}
	if (engine) {
		engine->drop();
	}
}
