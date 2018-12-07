#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <future>
#include "rtAudioData.h"
#include "rtTalkInterface.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
        class HTTPServerRequest;
        class HTTPServerResponse;
    }
}

namespace rt {

std::string ToANSI(const char *src);
void ServeText(Poco::Net::HTTPServerResponse& response, const std::string& data, int stat, const std::string& mimetype = "text/plain");
void ServeBinary(Poco::Net::HTTPServerResponse& response, RawVector<char>& data, const std::string& mimetype = "application/octet-stream");

struct TalkServerSettings
{
    int max_queue = 256;
    int max_threads = 8;
    uint16_t port = 8081;
};

class TalkServer
{
public:
    class Message
    {
    public:
        virtual ~Message() {}
        bool wait();
        bool isProcessing();

        std::atomic_bool ready = { false };
        std::ostream *respond_stream = nullptr;
        std::future<void> task;

    };
    using MessagePtr = std::shared_ptr<Message>;

    class TalkMessage : public Message
    {
    public:
        TalkParams params;
        std::string text;
    };

    class StopMessage : public Message
    {
    public:
    };

public:
    TalkServer(const TalkServer&) = delete;
    TalkServer& operator=(const TalkServer&) = delete;

    TalkServer();
    virtual ~TalkServer();
    virtual void setSettings(const TalkServerSettings& v);
    virtual bool start();
    virtual void stop();

    virtual void processMessages();
    virtual std::future<void> onTalk(const TalkParams& params, const std::string& text, std::ostream& os) = 0;
    virtual bool onStop() = 0;
    virtual bool ready() = 0;

    virtual void addMessage(MessagePtr mes);

private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    bool m_serving = true;
    TalkServerSettings m_settings;

    HTTPServerPtr m_server;
    std::mutex m_mutex;
    std::vector<MessagePtr> m_messages;
};

} // namespace rt
