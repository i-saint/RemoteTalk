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

    // no communication with server

    void clear();
    const std::vector<TalkerInfoImpl>& getTalkerList();

    void setMute(bool v);
    void setVolume(float v);
    void setSpeed(float v);
    void setPitch(float v);
    void setIntonation(float v);
    void setJoy(float v);
    void setAnger(float v);
    void setSorrow(float v);
    void setTalker(int v);


    // communicate with server 

    bool isServerAvailable();
    bool updateTalkerList();
    bool talk(const std::string& text, const std::function<void (const AudioData&)>& cb);
    bool stop();
    bool ready();

private:
    TalkClientSettings m_settings;

    TalkParams m_parmas;
    std::vector<TalkerInfoImpl> m_talkers;
    std::future<void> m_fut_stop;
    std::future<void> m_fut_talkers;
};

} // namespace rt
