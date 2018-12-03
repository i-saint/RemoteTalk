#pragma once
#include <string>

class rtvr2IController
{
public:
    struct TalkerInfo
    {
        std::string name;
    };

    struct Params
    {
        float volume = 1.0f;
        float speed = 1.0f;
        float pitch = 1.0f;
        float intonation = 1.0f;
    };

    struct Style
    {
        float joy = 0.0f;
        float anger = 0.0f;
        float sorrow = 0.0f;
    };

    virtual ~rtvr2IController() = 0;
    virtual void setTalker(int id) = 0;
    virtual void setVoiceParams(const Params& v) = 0;
    virtual void setStyleParams(const Style& v) = 0;
    virtual void talk(const std::string& text) = 0;

    virtual void dbgListWindows(std::vector<std::string>& dst) = 0;
};
