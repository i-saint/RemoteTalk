#pragma once
#include <string>

struct rtvr2TalkerInfo
{
    std::string name;
    int index = 0;
};

struct rtvr2Params
{
    float volume = 1.0f;
    float speed = 1.0f;
    float pitch = 1.0f;
    float intonation = 1.0f;
};

struct rtvr2Style
{
    float joy = 0.0f;
    float anger = 0.0f;
    float sorrow = 0.0f;
};

class rtvr2IController
{
public:
    virtual ~rtvr2IController() = 0;
    virtual void setTalker(int id) = 0;
    virtual void setVoiceParams(const rtvr2Params& v) = 0;
    virtual void setStyleParams(const rtvr2Style& v) = 0;
    virtual bool talk(const std::string& text) = 0;

    virtual void dbgListWindows(std::vector<std::string>& dst) = 0;
};
