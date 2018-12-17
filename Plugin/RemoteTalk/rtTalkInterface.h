#pragma once
#include <cstdint>
#include "rtAudioData.h"

#define rtInterfaceFuncName "rtGetTalkInterface"

namespace rt {

struct TalkParamInfo
{
    std::string name;
    float value = 0.0f, range_min = 0.0f, range_max = 0.0f;
};

struct CastInfo
{
    int id = 0;
    std::string name;
    std::vector<TalkParamInfo> params;
};
using CastList = std::vector<CastInfo>;

struct TalkParams
{
    struct Proxy
    {
        TalkParams *self;
        int index;

        operator float() const { return self->params[index]; };
        Proxy& operator=(float v)
        {
            self->param_flags = self->param_flags | (1 << index);
            self->params[index] = v;
            return *this;
        }
    };

    static const int MaxParams = 12;

    int mute = false; // as bool
    int force_mono = false; // as bool
    int cast = 0;
    int param_flags = 0;
    float params[MaxParams] = {};

    Proxy operator[](int i) { return { this, i }; }
    const Proxy operator[](int i) const { return { (TalkParams*)this, i }; }
    bool isSet(int i) const { return (param_flags & (1 << i)) != 0; }

    uint32_t hash() const
    {
        uint32_t ret = 0;
        for (int i = 0; i < MaxParams; ++i) {
            if (isSet(i)) {
                auto u = (uint32_t&)params[i];
                int s = (i * 3) % 32;
                ret ^= (u << s) | (u >> (32 - s));
            }
        }
        return ret;
    }
};

// one talk() will call this callback several times. last one has null data to notify end.
using TalkSampleCallback = void(*)(const AudioData& data, void *userdata);

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
    virtual const CastInfo* getCastInfo(int i) const = 0;
    virtual bool setText(const char *text) = 0;

    virtual bool ready() const = 0;
    virtual bool talk(TalkSampleCallback cb, void *userdata) = 0;
    virtual bool stop() = 0;
};

} // namespace rt
