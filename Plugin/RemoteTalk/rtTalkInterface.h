#pragma once
#include <cstdint>

namespace rt {

enum class TalkParamID
{
    Unknown,
    Volume,     // float
    Speed,      // float
    Pitch,      // float
    Intonation, // float
    Joy,        // float
    Anger,      // float
    Sorrow,     // float
    Talker,     // int
};

struct TalkerInfo
{
    const char *name = nullptr;
    int id = 0;
};

struct TalkSample
{
    const char *data;
    size_t size; // in byte
    int bits;
    int channels;
    int frequency;
};

class TalkInterface
{
public:

    // one talk() will call this callback several times. last one has null data to notify end.
    using SampleCallback = void(*)(const TalkSample *data, void *userdata);

    virtual ~TalkInterface() {}
    virtual void release() = 0;
    virtual const char* getClientName() const = 0;
    virtual int getPluginVersion() const = 0;
    virtual int getProtocolVersion() const = 0;

    virtual bool hasParam(TalkParamID pid) const = 0;
    virtual void setParam(TalkParamID pid, const void *value) = 0;
    virtual void getParam(TalkParamID pid, void *value) const = 0;
    virtual int getNumTalkers() const = 0;
    virtual void getTalkerInfo(int i, TalkerInfo *dst) const = 0;

    virtual bool talk(const char *text, SampleCallback cb, void *userdata) = 0;


    // utils
    void setParam(TalkParamID pid, float v) { setParam(pid, &v); }
    void setParam(TalkParamID pid, int v)   { setParam(pid, &v); }
    void getParam(TalkParamID pid, float& v) const { getParam(pid, &v); }
    void getParam(TalkParamID pid, int& v) const   { getParam(pid, &v); }
};

class AudioData;
TalkSample ToTalkSample(const AudioData& ad);
void ToAudioData(AudioData& dst, const TalkSample& ts);

} // namespace rt
