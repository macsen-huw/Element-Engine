#include "SoundComponent.hpp"
#include "AudioManager.hpp"
#include "Logger.hpp"
#include "core.hpp"

using namespace elmt;

SoundComponent::SoundComponent() : Component()
{
	typeName = "SoundComponent";
}

/*
Setup the component with custom gain and distance.
Use a negative value for gain/distance to avoid setting it
*/
SoundComponent::SoundComponent(const char* name, Entity* entity, const char* fileName, SoundInfo info) : Component(name, entity)
{
	
	sound.ma_sound_data = (ma_sound*)malloc(sizeof(ma_sound));
	sound.managed = false;
	sound.isSpatial = true;

	

	playing = false;

	hasLoadedSound = false;
	this->fileName = "";
	setSound(fileName);

	setInfo(info);

	typeName = "SoundComponemt";
}

SoundComponent::SoundComponent(const char* name, Entity* entity, const char* fileName) : SoundComponent(name, entity, fileName, SoundInfo())
{
}

/*
Called automatically by miniaudio every time the sound ends
*/
void _sc_soundEndCallback(void* component, ma_sound* sound_data ) {
	// Bit a hacky workaround for playing attribute being private
	SoundComponent* soundComponent = (SoundComponent*)component;
	Sound* sound = core::getAudioManager()->getSoundByData(sound_data);
	core::getAudioManager()->removePlayingSound( sound );
	soundComponent->Stop();
}

/*
Set up the sound
This is automatically called in the constructor
*/
int SoundComponent::setSound(const char* fileName)
{
	if (playing) {
		Stop();
	}
	if (hasLoadedSound) {
		ma_sound_uninit(sound.ma_sound_data);
	}

	hasLoadedSound = false;
	this->fileName = fileName;

	// Load sound file
	// TODO flag support
	// TODO load check
	ma_uint32 flags;
	if (!info.pitchShiftEnabled) {
		flags &= MA_SOUND_FLAG_NO_PITCH;
	}

	ma_result result = ma_sound_init_from_file(&(core::getAudioManager()->engine), this->fileName.c_str(), flags, nullptr, NULL, sound.ma_sound_data);
	if (result != MA_SUCCESS) {
		Logger::Print("Failed to initialise sound from file: \"" + this->fileName + "\"", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
		return 1;
	}

	setInfo(info);

	

	hasLoadedSound = true;
	ma_sound_set_end_callback(sound.ma_sound_data, &_sc_soundEndCallback, this);
	return 0;
}

void SoundComponent::setInfo(SoundInfo newInfo) {
	info = newInfo;
	core::getAudioManager()->setInfo(&sound, info);

}




bool SoundComponent::Update()
{
	bool res = Component::Update();
	if (!res) return false;

	if (playing) {
		updateTransform();
		
	}
	return true;

}

/*
Update the sound's position to fit with it's entity
This is called automatically when required but can also be called manually if needed
*/
int SoundComponent::updateTransform()
{
	if (!entity) {
		Logger::Print("Tried to update sound transform for SoundComponent \"" + name + "\", but it had no attached Entity",
			LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
		return 1;
	}

	ma_sound_set_position(sound.ma_sound_data, entity->pos.x, entity->pos.y, entity->pos.z);

	glm::vec3 vel = (float)core::getDeltaTime() * entity->posChange;
	ma_sound_set_velocity(sound.ma_sound_data, vel.x, vel.y, vel.z);


	return 0;
}

/*
Actually play the sound associated with this component
*/
int SoundComponent::Play()
{
	if (!entity) {
		Logger::Print("Tried to play a sound with SoundComponent \"" + name + "\", but it had no attached Entity",
			LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
		return 1;
	}

	if (!hasLoadedSound) {
		Logger::Print("Tried to play a sound with SoundComponent \"" + name + "\", but it had no valid loaded sound \n"
			"(filename was: \"" + fileName + "\"",
			LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
		return 2;
	}

	if (!playing) {
		updateTransform();
		core::getAudioManager()->addPlayingSound(&sound);
		ma_sound_start(sound.ma_sound_data);
	}

	playing = true;
	return 0;
}

/*
Stop the sound. This method is safe to call if the sound is not playing.
*/
int SoundComponent::Stop() {
	if (!hasLoadedSound) {
		Logger::Print("Tried to stop playback with SoundComponent \"" + name + "\", but it had no valid loaded sound \n"
			"(filename was: \"" + fileName + "\"",
			LOGCAT::CAT_CORE, LOGSEV::SEV_ERROR);
		return 2;
	}

	bool soundFinished = ma_sound_at_end(sound.ma_sound_data);
	playing = false;
	if (soundFinished) {
		// This branch will be hit by the end sound callback
	}
	else {
		ma_sound_stop(sound.ma_sound_data);
	}
	ma_sound_seek_to_pcm_frame(sound.ma_sound_data, 0);

	return 0;
}


SoundComponent::~SoundComponent()
{
	if (core::getIsSetup()) {
		// Unitilise whatever sound we had
		if (hasLoadedSound) {
			ma_sound_uninit(sound.ma_sound_data);
		}
	}

	if (sound.ma_sound_data) {
		free(sound.ma_sound_data);
	}
	
	
}

void SoundComponent::clone(Component*& clonePointer, Entity* entityToAttach)
{
	clonePointer = new SoundComponent(name.c_str(), entity, fileName.c_str(), info);
	Logger::Print("Cloned SoundComponent " + name + ", UUID " + clonePointer->getName(), LOGCAT::CAT_CORE, LOGSEV::SEV_INFO | LOGSEV::SEV_TRACE);
}
