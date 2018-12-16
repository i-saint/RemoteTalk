#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

struct rtAsyncBase
{
    virtual ~rtAsyncBase() {}
    virtual bool isValid() = 0;
    virtual bool isFinished() = 0;
    virtual void wait() = 0;
};

template<class T>
struct rtAsync : public rtAsyncBase
{
    std::future<T> task;

    bool isValid() override
    {
        return task.valid();
    }

    bool isFinished() override
    {
        return !task.valid() || task.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

    void wait() override
    {
        if (task.valid())
            task.wait();
    }

    T get()
    {
        return task.get();
    }
};

class rtHTTPClient
{
public:
    rtHTTPClient(const char *server, uint16_t port);
    ~rtHTTPClient();
    void release();

    rtAsync<bool>& updateServerStats();
    const rt::TalkServerStats& getServerStats() const;

    bool isReady();
    rtAsync<bool>& talk(const rt::TalkParams& params, const std::string& text);
    rtAsync<bool>& stop();
    rtAsync<bool>& exportWave(const std::string& path);
    rtAsync<bool>& exportOgg(const std::string& path, const rt::OggSettings& settings);

    void wait();
    const rt::AudioData& syncBuffers();
    const rt::AudioData& getBuffer();

private:
    rt::TalkClientSettings m_settings;
    rt::TalkClient m_client;

    rt::TalkServerStats m_server_stats;
    rt::AudioData m_buf_receiving;
    rt::AudioData m_buf_public;
    std::mutex m_mutex;
    rtAsync<bool> m_task_stats;
    rtAsync<bool> m_task_talk;
    rtAsync<bool> m_task_stop;
    rtAsync<bool> m_task_export;
    bool m_processing = false;
};


