#pragma once
#include <string>
#include <memory>
#include <mutex>
#include "rtAudioData.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
        class HTTPServerRequest;
        class HTTPServerResponse;
    }
}

namespace rt {

struct ServerSettings
{
    int max_queue = 256;
    int max_threads = 8;
    uint16_t port = 8081;
};

class Server
{
public:
    virtual ~Server();
    virtual void setSettings(const ServerSettings& v);
    virtual bool start();
    virtual void stop();

    virtual bool onSetParam(const std::string& name, const std::string& value) = 0;
    virtual std::future<AudioDataPtr> onTalk(const std::string& text) = 0;

    void serveText(Poco::Net::HTTPServerResponse &response, const char* text, int stat = 200);

private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

private:
    bool m_serving = true;
    ServerSettings m_settings;

    HTTPServerPtr m_server;
    std::mutex m_mutex;
};

} // namespace rt
