#pragma once
#include <cstdint>

#define rtInterfaceFuncName "rtGetTalkInterface"

namespace rt {

#define rtEachTalkParams(Body)\
    Body(mute) Body(force_mono)\
    Body(volume) Body(speed) Body(pitch) Body(intonation) Body(alpha)\
    Body(normal) Body(joy) Body(anger) Body(sorrow) Body(cast)

struct TalkParams
{
    struct Flags
    {
        uint32_t mute : 1;
        uint32_t force_mono : 1;
        uint32_t volume : 1;
        uint32_t speed : 1;
        uint32_t pitch : 1;     // 5
        uint32_t intonation : 1;
        uint32_t alpha : 1;
        uint32_t normal : 1;
        uint32_t joy : 1;
        uint32_t anger : 1;     // 10
        uint32_t sorrow : 1;
        uint32_t cast : 1;
    };
    
    Flags flags = {};
    int mute = false; // as bool
    int force_mono = false; // as bool
    float volume = 1.0f;
    float speed = 1.0f;
    float pitch = 1.0f;
    float intonation = 1.0f;
    float alpha = 1.0f;
    float normal = 0.0f;
    float joy = 0.0f;
    float anger = 0.0f;
    float sorrow = 0.0f;
    int cast = 0;

#define Set(V) flags.V = 1; ##V = v;
    void setMute(bool v) { Set(mute); }
    void setForceMono(bool v) { Set(force_mono); }
    void setVolume(float v) { Set(volume); }
    void setSpeed(float v) { Set(speed); }
    void setPitch(float v) { Set(pitch); }
    void setIntonation(float v) { Set(intonation); }
    void setAlpha(float v) { Set(alpha); }
    void setNormal(float v) { Set(normal); }
    void setJoy(float v) { Set(joy); }
    void setAnger(float v) { Set(anger); }
    void setSorrow(float v) { Set(sorrow); }
    void setCast(int v) { Set(cast); }
#undef Set
};

struct CastInfo
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

class TalkInterface
{
public:
    virtual ~TalkInterface() {}
    virtual void release() = 0;
    virtual const char* getClientName() const = 0;
    virtual int getPluginVersion() const = 0;
    virtual int getProtocolVersion() const = 0;

    virtual bool getParams(TalkParams& params) const = 0;
    virtual bool setParams(const TalkParams& params) = 0;
    virtual int getNumCasts() const = 0;
    virtual bool getCastInfo(int i, CastInfo *dst) const = 0;
    virtual bool setText(const char *text) = 0;

    virtual bool ready() const = 0;
    virtual bool talk(TalkSampleCallback cb, void *userdata) = 0;
    virtual bool stop() = 0;
};

} // namespace rt
