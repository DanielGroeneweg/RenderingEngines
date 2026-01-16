#include "AudioSystem.h"

bool AudioSystem::Init() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
        return false;
    return true;
}

void AudioSystem::Shutdown() {
    if (musicInitialized) {
        ma_sound_uninit(&music);
    }
    ma_engine_uninit(&engine);
}

bool AudioSystem::PlayMusic(const std::string &filepath) {
    if (musicInitialized) {
        ma_sound_uninit(&music);   // <-- uninit old sound before creating a new one
        musicInitialized = false;
    }

    if (ma_sound_init_from_file(&engine, filepath.c_str(), MA_SOUND_FLAG_STREAM, NULL, NULL, &music) != MA_SUCCESS)
        return false;

    ma_sound_set_looping(&music, MA_TRUE);
    ma_sound_start(&music);
    musicInitialized = true;
    return true;
}

void AudioSystem::StopMusic() {
    if (musicInitialized) {
        ma_sound_stop(&music);
        ma_sound_uninit(&music);  // <-- fully uninit
        musicInitialized = false;
    }
}

void AudioSystem::SetVolume(float volume) {
    if (musicInitialized) {
        ma_sound_set_volume(&music, volume);
    }
}