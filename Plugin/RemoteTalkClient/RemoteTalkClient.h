#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

class rtHTTPClient
{
public:
    using async = std::future<void>;

    rtHTTPClient(const char *server, uint16_t port);
    ~rtHTTPClient();
    void release();

    async& updateServerStats();
    const rt::TalkServerStats& getServerStats() const;

    bool isReady();
    async& talk(const rt::TalkParams& params, const std::string& text);
    async& stop();

    void wait();
    rt::AudioData* syncBuffers();
    rt::AudioData* getBuffer();

private:
    rt::TalkClientSettings m_settings;
    rt::TalkClient m_client;

    rt::TalkServerStats m_server_stats;
    rt::AudioData m_buf_receiving;
    rt::AudioData m_buf_public;
    std::mutex m_mutex;
    async m_task_stats;
    async m_task_talk;
    async m_task_stop;
    bool m_processing = false;
};


