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

        operator float() const;
        Proxy& operator=(float v);
    };

    static const int MaxParams = 12;

    int mute = false; // as bool
    int force_mono = false; // as bool
    int cast = 0;
    int param_flags = 0;
    float params[MaxParams] = {};

    Proxy operator[](int i);
    const Proxy operator[](int i) const;
    bool isSet(int i) const;
    uint32_t hash() const;
};


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

    virtual bool isReady() const = 0;
    virtual bool isPlaying() const = 0;
    virtual bool talk() = 0;
    virtual bool stop() = 0;
};

} // namespace rt
