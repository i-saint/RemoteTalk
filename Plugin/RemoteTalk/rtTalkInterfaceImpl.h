#pragma once
#include "rtTalkInterface.h"

namespace rt {
    
class AudioData;
TalkSample ToTalkSample(const AudioData& ad);
void ToAudioData(AudioData& dst, const TalkSample& ts);

struct CastInfoImpl
{
    int id = 0;
    std::string name;
    std::vector<std::string> param_names;

    void fromCastInfo(const CastInfo& src);
    CastInfo toCastInfo();
};
using CastList = std::vector<CastInfoImpl>;

} // namespace rt
