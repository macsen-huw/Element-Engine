#pragma once
#include <vector>
#include "miniaudio.h"
#include "../external/glm-0.9.7.1/glm/glm.hpp"

namespace elmt {


	enum ATTENUATION_MODEL {
		ATT_NONE,
		ATT_INVERSE,
		ATT_LINEAR,
		ATT_EXPONENTIAL
	};

	/*
	Info used for setting or updating sound properties
	Set a property to -1.0 to not apply it to the sound
	*/
	struct SoundInfo {
		ATTENUATION_MODEL attenuation = ATT_INVERSE; // How sound fades over distance
		float rolloff = -1.0; 
		float minGain = -1.0;
		float maxGain = -1.0;
		float minDistance = -1.0; // Min dis
		float maxDistance = -1.0;

		float dopplerFactor = 1.0; // Doppler factor
		float fadeTime = -1.0; // Fade in/out in seconds

		float pitch = -1.0;
		bool pitchShiftEnabled = false; // This must be set if you want to change the pitch of the sound

		float pan = 0.0;
		bool isLooping = false;
	};


	/*
	Stores info about a sound, mainly just a wrapper over ma_sound
	In most cases, this should not be modified directly
	*/
	struct Sound {
		// Unique ID for a sound
		size_t soundID = 0;
		ma_sound* ma_sound_data = nullptr;
		// Info about a sound
		SoundInfo info;

		bool playing = false;
		bool isSpatial = false;
		// If true, sound is handled by the Audio Manager and is cleaned up when sound playing ends
		bool managed = false;
	};



	// Forward declaration
	class SoundComponent;

	class AudioManager
	{
	// Attributes
	private:
		friend class SoundComponent;
		float globalVolume = 1.0;
		// All sounds currently playing
		std::vector<Sound*> playingSounds;
		size_t nextSoundID = 1;

	public:
		ma_engine engine;

	// Methods
	private:
		
		

	public:

		AudioManager();
		~AudioManager();

		size_t getNextSoundID() { return nextSoundID++; };

		int playSound(Sound* sound, glm::vec3 pos, SoundInfo& info);

		int playSound(Sound* sound, glm::vec3 pos) {
            auto soundInfo = SoundInfo();
            return playSound(sound, pos, soundInfo);
        }

		int playSound(Sound* sound, SoundInfo& info) {
            auto soundInfo = SoundInfo();
            return playSound(sound, glm::vec3(0.0, 0.0, 0.0), soundInfo);
        };

		int playSound(Sound * sound);

		Sound* playSound(const char* filePath, glm::vec3 pos, ma_sound_flags miniaudioFlags, SoundInfo & info);
		Sound* playSound(const char* filePath, SoundInfo& info) { return playSound(filePath, glm::vec3(0, 0, 0), MA_SOUND_FLAG_NO_SPATIALIZATION, info); };
		Sound* playSound(const char* filePath) {
            auto soundInfo = SoundInfo();
            return playSound(filePath, soundInfo);
        };
		

		// Stop a sound which is currently playing.
		// Returns 0 if successful, 1 otherwise (most likely because the sound was not in the list of current sounds)
		int stopSound(Sound* sound);

		// Add a sound to the list of currently playing sounds. Prefer to not call this manually
		int addPlayingSound(Sound* sound);
		// Remove a sound from the list of currently playing sounds. Prefer to not call this manually
		int removePlayingSound(Sound* sound);

		float getGlobalVolume() { return globalVolume; }
		void setGlobalVolume(float val);
		void updateSound(Sound* sound);

		// Find a sound based on it's internal ma data, returns nullptr if no sound was found
		Sound* getSoundByData(ma_sound* sound);
		// Find a sound based on it's ID, returns nullptr if no sound was found
		Sound* getSoundByID(size_t id);

		// Update information about how the sound should be played
		void setInfo(Sound* sound, SoundInfo& info);

		const std::vector<Sound*>& getPlayingSounds() { return playingSounds; }

		

	};


}