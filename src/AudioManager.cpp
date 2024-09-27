
#include "AudioManager.hpp"
#include "Logger.hpp"
#include "core.hpp"
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.c"

//#include "Logger.hpp"

#include <stdio.h>

using namespace elmt;


int AudioManager::addPlayingSound(Sound* sound)
{
    auto soundIndex = std::find(playingSounds.begin(), playingSounds.end(), sound);
    if (soundIndex != playingSounds.end()) {
        Logger::Print("Attempted to add sound to playing sound list, but it was already added", LOGCAT::CAT_AUDIO, LOGSEV::SEV_WARNING);
        return 1;
    }
    playingSounds.push_back(sound);
    updateSound(sound);
    return 0;
}

int AudioManager::removePlayingSound(Sound* sound)
{
    auto soundIndex = std::find(playingSounds.begin(), playingSounds.end(), sound);
    if (soundIndex != playingSounds.end()) {
        playingSounds.erase(soundIndex);
    }
    else {
        Logger::Print("Attempted to remove sound from currently playing sound list, but it was not found", LOGCAT::CAT_AUDIO, LOGSEV::SEV_WARNING);
        return 1;
    }

    return 0;
}

void AudioManager::setGlobalVolume(float val)
{
    globalVolume = std::fmaxf(val, 0.0);

    if (val < 0) {
        Logger::Print("Attempted to set global volume to value < 0 (" + std::to_string(val) + ")", LOGCAT::CAT_AUDIO, LOGSEV::SEV_WARNING);
    }

    for (Sound* sound : playingSounds) {
        updateSound(sound);
    }
}

void AudioManager::updateSound(Sound* sound)
{
    ma_sound_set_volume(sound->ma_sound_data, globalVolume);
}

Sound* AudioManager::getSoundByData(ma_sound* sound)
{
    for (Sound* snd : playingSounds) {
        if (snd->ma_sound_data == sound) {
            return snd;
        }
    }
    return nullptr;
}

Sound* AudioManager::getSoundByID(size_t id)
{
    for (Sound* snd : playingSounds) {
        if (snd->soundID == id) {
            return snd;
        }
    }
    return nullptr;
}

void AudioManager::setInfo(Sound* sound, SoundInfo& info)
{
    sound->info = info;
    if (sound->isSpatial) {

        if (info.minGain >= 0.0) { ma_sound_set_min_gain(sound->ma_sound_data, info.minGain); }
        if (info.maxGain >= 0.0) { ma_sound_set_max_gain(sound->ma_sound_data, info.maxGain); }
        if (info.minDistance >= 0.0) { ma_sound_set_min_distance(sound->ma_sound_data, info.minDistance); }
        if (info.maxDistance >= 0.0) { ma_sound_set_min_distance(sound->ma_sound_data, info.maxDistance); }
        if (info.rolloff >= 0.0) { ma_sound_set_rolloff(sound->ma_sound_data, info.rolloff); }
        if (info.dopplerFactor >= 0.0) { ma_sound_set_doppler_factor(sound->ma_sound_data, info.dopplerFactor); }

        switch (info.attenuation)
        {
        case ATT_NONE:
            ma_sound_set_attenuation_model(sound->ma_sound_data, ma_attenuation_model_none);
            break;
        case ATT_INVERSE:
            ma_sound_set_attenuation_model(sound->ma_sound_data, ma_attenuation_model_inverse);
            break;
        case ATT_LINEAR:
            ma_sound_set_attenuation_model(sound->ma_sound_data, ma_attenuation_model_linear);
            break;
        case ATT_EXPONENTIAL:
            ma_sound_set_attenuation_model(sound->ma_sound_data, ma_attenuation_model_exponential);
            break;
        }

    }
    else {
        if (info.fadeTime >= 0.0) { ma_sound_set_fade_in_milliseconds(sound->ma_sound_data, 0, globalVolume, info.fadeTime * 1000); }
        if (info.pitch >= 0.0 && info.pitchShiftEnabled) { ma_sound_set_pitch(sound->ma_sound_data, info.pitch); }
        ma_sound_set_pan(sound->ma_sound_data, info.pan);
    }

    ma_sound_set_looping(sound->ma_sound_data, info.isLooping);
}

AudioManager::AudioManager()
{
    ma_result result;

    result = ma_engine_init(NULL, &engine);



    if (result != MA_SUCCESS) {
        Logger::Print("AudioManager.playSound: Failed to setup audio, error code: " + result, LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
    }
}

AudioManager::~AudioManager()
{
    ma_engine_uninit(&engine);
}


/*
Called automatically by miniaudio every time the sound ends
*/
void _am_soundEndCallback(void* sound_void, ma_sound*) {
    // TODO fix this (causes all sound playback to fail)
    //ma_sound_uninit(sound);
    //free(sound);
    Sound* sound = (Sound*)sound_void;
    sound->playing = false;
    core::getAudioManager()->removePlayingSound((Sound*)sound);

}



/*
Play a sound at a given spatial location
*/
int AudioManager::playSound(Sound* sound, glm::vec3 pos, SoundInfo& info) {

    if (pos.length() > 0.0) {
        ma_sound_set_position(sound->ma_sound_data, pos.x, pos.y, pos.z);
    }

    //Use MA_SOUND_FLAG_NO_SPATIALIZATION to explicitily disable spatialisation
    ma_result result = ma_sound_start(sound->ma_sound_data);

    if (result != MA_SUCCESS) {
        Logger::Print("AudioManager.playSound: Failed to play sound, error code: " + result, LOGCAT::CAT_LOADING, LOGSEV::SEV_ALL);
        return 1;
    }

    addPlayingSound(sound);

    return 0;
}



/*
Play a sound from a file, with given spatial location and flags
*/
Sound* AudioManager::playSound(const char* filePath, glm::vec3 pos, ma_sound_flags miniaudioFlags, SoundInfo& info) {
    // This is freed in _am_soundEndCallback
    ma_sound* sound_data = (ma_sound*)malloc(sizeof(ma_sound));
    Sound* sound = (Sound*)malloc(sizeof(Sound));
    sound->soundID = getNextSoundID();
    sound->ma_sound_data = sound_data;
    sound->managed = true;
    sound->isSpatial = false;

    //Use MA_SOUND_FLAG_NO_SPATIALIZATION to explicitly prevent spatial audio
    ma_result result = ma_sound_init_from_file(&engine, filePath, miniaudioFlags, NULL, NULL, sound_data);
    ma_sound_set_end_callback(sound_data, &_am_soundEndCallback, sound);

    setInfo(sound, info);

    if (result != MA_SUCCESS) {
        Logger::Print("AudioManager.playSound: Failed to load sound from file: \"" + std::string(filePath) + std::string("\", error code: " + result), LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return nullptr;
    }


    if (pos.length() > 0.0) {
        ma_sound_set_position(sound_data, pos.x, pos.y, pos.z);
    }

    int playRes = playSound(sound, pos, info);
    if (playRes) {
        Logger::Print("AudioManager.playSound: Failed to play sound from file: \"" + std::string(filePath) + "\"", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return nullptr;
    }



    return sound;
}






int AudioManager::stopSound(Sound* sound) {
    auto soundIndex = std::find(playingSounds.begin(), playingSounds.end(), sound);
    if (soundIndex != playingSounds.end()) {
        auto res = ma_sound_stop(sound->ma_sound_data);
        if (res != MA_SUCCESS) {
            Logger::Print("AudioManager.stopSound: Failed to stop sound (miniaudio returned " + std::to_string(res) + ")", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
            return 1;
        }
        if (sound->managed) {
            free(sound);
        }
        return 0;
    }
    else {
        Logger::Print("AudioManager.stopSound: Failed to stop sound, as it was not playing", LOGCAT::CAT_LOADING, LOGSEV::SEV_ERROR);
        return 1;
    }
    
}
