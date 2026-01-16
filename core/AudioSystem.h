#pragma once
#include <miniaudio.h>
#include <string>

class AudioSystem {
public:
    bool Init();
    void Shutdown();

    bool PlayMusic(const std::string& filepath);
    void StopMusic();
    void SetVolume(float volume);

private:
    ma_engine engine;
    ma_sound music;
    bool musicInitialized = false;
};