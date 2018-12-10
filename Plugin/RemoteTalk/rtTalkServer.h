#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <future>
#include "rtAudioData.h"
#include "rtTalkInterfaceImpl.h"

namespace Poco {
    namespace Net {
        class HTTPServer;
        class HTTPServerRequest;
        class HTTPServerResponse;
    }
}

namespace rt {

void ServeText(Poco::Net::HTTPServerResponse& response, const std::string& data, int stat, const std::string& mimetype = "text/plain");
void ServeBinary(Poco::Net::HTTPServerResponse& response, RawVector<char>& data, const std::string& mimetype = "application/octet-stream");

struct TalkServerSettings
{
    int max_queue = 256;
    int max_threads = 8;
    uint16_t port = 8081;
};

struct TalkServerStats
{
    std::string host;
    int plugin_version = 0;
    int protocol_version = 0;
    TalkParams params;
    CastList casts;

    std::string to_json();
    bool from_json(const std::string& str);
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

        std::atomic_bool handled = { false };
        std::ostream *respond_stream = nullptr;
        std::future<void> task;

    };
    using MessagePtr = std::shared_ptr<Message>;

    class TalkMessage : public Message
    {
    public:
        TalkParams params;
        std::string text;

        std::string to_json();
        bool from_json(const std::string& str);
    };

    class StopMessage : public Message
    {
    public:
    };

    class StatsMessage : public Message
    {
    public:
        TalkServerStats stats;

        std::string to_json();
        bool from_json(const std::string& str);
    };

#ifdef rtDebug
    class DebugMessage : public Message
    {
    public:
        std::map<std::string, std::string> params;

        std::string to_json();
        bool from_json(const std::string& str);
    };
#endif

public:
    TalkServer(const TalkServer&) = delete;
    TalkServer& operator=(const TalkServer&) = delete;

    TalkServer();
    virtual ~TalkServer();
    virtual void setSettings(const TalkServerSettings& v);
    virtual bool start();
    virtual void stop();
    virtual bool isRunning() const;

    virtual void processMessages();
    virtual bool onStats(StatsMessage& mes) = 0;
    virtual bool onTalk(TalkMessage& mes) = 0;
    virtual bool onStop(StopMessage& mes) = 0;
    virtual bool ready() = 0;

#ifdef rtDebug
    virtual bool onDebug(DebugMessage& mes) { return true; }
#endif

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
