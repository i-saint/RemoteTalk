#pragma once
#include "rtTalkInterface.h"

namespace rt {
    
class AudioData;
TalkSample ToTalkSample(const AudioData& ad);
void ToAudioData(AudioData& dst, const TalkSample& ts);

struct AvatorInfoImpl
{
    int id = 0;
    std::string name;
};
using AvatorList = std::vector<AvatorInfoImpl>;

} // namespace rt
