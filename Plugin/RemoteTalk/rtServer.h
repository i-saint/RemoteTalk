#pragma once
#include <string>
#include <vector>
#include <map>
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
        std::atomic_bool ready = { false };
    };
    using MessagePtr = std::shared_ptr<Message>;

    class ParamMessage : public Message
    {
    public:
        std::vector<std::pair<std::string, std::string>> params;
    };

    class TalkMessage : public ParamMessage
    {
    public:
        std::string text;
        AudioDataPtr data;
    };

public:
    virtual ~TalkServer();
    virtual void setSettings(const TalkServerSettings& v);
    virtual bool start();
    virtual void stop();

    virtual void processMessages();
    virtual bool onSetParam(const std::string& name, const std::string& value) = 0;
    virtual bool onTalk(const std::string& text) = 0;

    void addMessage(MessagePtr mes);

private:
    using HTTPServerPtr = std::shared_ptr<Poco::Net::HTTPServer>;
    using lock_t = std::unique_lock<std::mutex>;

    bool m_serving = true;
    TalkServerSettings m_settings;

    HTTPServerPtr m_server;
    std::mutex m_mutex;
    std::vector<MessagePtr> m_messages;
};


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
    ~TalkClient();
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
