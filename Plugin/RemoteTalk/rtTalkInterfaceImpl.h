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

template<class T> std::string to_json(const T& v);
template<class T> void from_json(T& v, const std::string& json);

} // namespace rt
