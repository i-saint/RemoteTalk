#pragma once
#include "rtAudioData.h"

namespace rt {

struct OggSettings
{
    float quality = 1.0f;
};


bool ExportWave(const AudioData& ad, std::ostream& os);
bool ExportWave(const AudioData& ad, const char* path);
bool ExportOgg(const AudioData& ad, std::ostream& os, const OggSettings& settings = {});
bool ExportOgg(const AudioData& ad, const char* path, const OggSettings& settings = {});

} // namespace rt
