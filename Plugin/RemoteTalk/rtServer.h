#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <future>
#include "rtAudioData.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
        class HTTPServerRequest;
        class HTTPServerResponse;
    }
}

namespace rt {

std::string ToANSI(const char *src);

struct ServerSettings
{
    int max_queue = 256;
    int max_threads = 8;
    uint16_t port = 8081;
};

class Server
{
public:
    class Message
    {
    public:
        virtual ~Message() {}
        bool wait();
        std::atomic_bool ready = { false };
    };
    using MessagePtr = std::shared_ptr<Message>;

    class ParamMessage : public Message
    {
    public:
        std::vector<std::pair<std::string, std::string>> params;
    };

    class TalkMessage : public Message
    {
    public:
        std::string text;
        AudioDataPtr data;
    };

public:
    virtual ~Server();
    virtual void setSettings(const ServerSettings& v);
    virtual bool start();
    virtual void stop();

    virtual void processMessages();
    virtual bool onSetParam(const std::string& name, const std::string& value) = 0;
    virtual bool onTalk(const std::string& text) = 0;

    void addMessage(MessagePtr mes);
    void serveText(Poco::Net::HTTPServerResponse &response, const char* text, int stat = 200);


private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    bool m_serving = true;
    ServerSettings m_settings;

    HTTPServerPtr m_server;
    std::mutex m_mutex;
    std::vector<MessagePtr> m_messages;
};

} // namespace rt
