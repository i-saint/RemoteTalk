#pragma once
#include "rtAudioData.h"

namespace rt {

enum class BitrateMode
{
    CBR,
    VBR,
};

struct OggSettings
{
    BitrateMode bitrate_mode = BitrateMode::VBR;
    int target_bitrate = 128 * 1000;
};


bool ExportWave(const AudioData& ad, const std::wstring& path);
bool ExportOgg(const AudioData& ad, const std::wstring& path, const OggSettings& settings);

} // namespace rt
