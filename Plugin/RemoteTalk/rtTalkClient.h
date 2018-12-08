#pragma once
#include "rtTalkInterfaceImpl.h"
#include "rtTalkServer.h"

namespace rt {

struct TalkClientSettings
{
    std::string server;
    uint16_t port;
    int timeout_ms;

    TalkClientSettings(const std::string& s= "127.0.0.1", uint16_t p = 8081, int ms=30000)
    : server(s), port(p), timeout_ms(ms)
    {}
};

class TalkClient
{
public:
    TalkClient(const TalkClientSettings& settings);
    virtual ~TalkClient();

    // communicate with server 

    bool isServerAvailable();
    bool getParams(TalkParams& params, AvatorList& avators);
    bool talk(const TalkParams& params, const std::string& text, const std::function<void (const AudioData&)>& cb);
    bool stop();
    bool ready();

private:
    TalkClientSettings m_settings;
};

} // namespace rt
