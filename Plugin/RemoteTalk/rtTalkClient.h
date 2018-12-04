#pragma once
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
    void setParam(const std::string& name, const std::string& value);
    void setText(const std::string& text);
    std::future<AudioDataPtr> send();

private:
    TalkClientSettings m_settings;
    std::map<std::string, std::string> m_params;
    std::string m_text;
    AudioDataPtr m_audio_data;
};

} // namespace rt
