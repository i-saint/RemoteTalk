#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

struct rtAsyncBase
{
    virtual ~rtAsyncBase() {}
    virtual bool isValid() = 0;
    virtual bool isFinished() = 0;
    virtual bool wait(int timeout_ms = 0) = 0;
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

    bool wait(int timeout_ms = 0) override
    {
        if (!task.valid())
            return false;

        if (timeout_ms <= 0) {
            task.wait();
            return true;
        }
        else {
            return task.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::ready;
        }
    }

    T get()
    {
        return task.get();
    }
};

class rtHTTPClient
{
public:
    rtHTTPClient();
    ~rtHTTPClient();
    void release();
    void reset(const char *address, uint16_t port);

    rtAsync<bool>& updateServerStats();
    const rt::TalkServerStats& getServerStats() const;

    bool isReady();
    rtAsync<bool>& play(const rt::TalkParams& params, const std::string& text);
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


