#pragma once
#include "rtTalkInterface.h"
#include "rtTalkServer.h"

namespace rt {

struct TalkClientSettings
{
    std::string server = "127.0.0.1";
    uint16_t port = 8081;
    int timeout_ms = 30000;
};

class TalkClient
{
public:
    TalkClient(const TalkClientSettings& settings);
    virtual ~TalkClient();
    void clear();

    void setSilence(bool v);
    void setForce(bool v);
    void setVolume(float v);
    void setSpeed(float v);
    void setPitch(float v);
    void setIntonation(float v);
    void setJoy(float v);
    void setAnger(float v);
    void setSorrow(float v);
    void setText(const std::string& text);
    bool send(const std::function<void (const AudioData&)>& cb);

private:
    TalkClientSettings m_settings;

    TalkParams m_parmas;
    std::string m_text;
};

} // namespace rt
