#pragma once
#include "rtTalkServer.h"

namespace rt {

class TalkReceiver
{
public:
    virtual ~TalkReceiver();
    virtual void setSettings(const TalkServerSettings& v);
    virtual bool start();
    virtual void stop();

private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    bool m_serving = true;
    TalkServerSettings m_settings;

    HTTPServerPtr m_server;
    std::mutex m_mutex;
};

} // namespace rt
