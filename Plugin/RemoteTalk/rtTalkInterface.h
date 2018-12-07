#pragma once
#include <cstdint>
#include "rtFoundation.h"

namespace rt {

#define rtEachTalkParams(Body)\
    Body(mute) Body(volume) Body(speed) Body(pitch) Body(intonation) Body(joy) Body(anger) Body(sorrow)

union TalkParamFlags
{
    struct {
        uint32_t mute : 1;
        uint32_t volume : 1;
        uint32_t speed : 1;
        uint32_t pitch : 1;
        uint32_t intonation : 1;
        uint32_t joy : 1;
        uint32_t anger : 1;
        uint32_t sorrow : 1;
        uint32_t talker : 1;
    } fields;
    uint32_t bits;

    TalkParamFlags() : bits(0) {}
    TalkParamFlags(uint32_t v) : bits(v) {}

    static TalkParamFlags none() { return TalkParamFlags(0); }
    static TalkParamFlags all() { return TalkParamFlags(~0u); }
};

struct TalkParams
{
    TalkParamFlags flags;
    bool mute;
    float volume;
    float speed;
    float pitch;
    float intonation;
    float joy;
    float anger;
    float sorrow;
    int talker;
};

struct TalkerInfo
{
    int id = 0;
    const char *name = nullptr;
};

struct TalkSample
{
    const char *data;
    int size; // in byte
    int bits;
    int channels;
    int frequency;
};

// one talk() will call this callback several times. last one has null data to notify end.
using TalkSampleCallback = void(*)(const TalkSample *data, void *userdata);


#pragma warning(push)
#pragma warning(disable:4100)
class TalkInterface
{
public:
    virtual ~TalkInterface() {}
    virtual void release() = 0;
    virtual const char* getClientName() const = 0;
    virtual int getPluginVersion() const { return rtPluginVersion; }
    virtual int getProtocolVersion() const { return rtProtocolVersion; }

    virtual void getParams(TalkParams& params) const = 0;
    virtual int getNumTalkers() const { return 0; }
    virtual bool getTalkerInfo(int i, TalkerInfo *dst) const { return false; }

    virtual bool talk(const TalkParams& params, const char *text, TalkSampleCallback cb, void *userdata) = 0;
    virtual bool stop() = 0;
    virtual bool ready() const = 0;
};
#pragma warning(pop)

class AudioData;
TalkSample ToTalkSample(const AudioData& ad);
void ToAudioData(AudioData& dst, const TalkSample& ts);

struct TalkerInfoImpl
{
    int id = 0;
    std::string name;
};

} // namespace rt
