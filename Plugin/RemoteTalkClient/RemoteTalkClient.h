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

    async& updateServerStatus();
    const rt::TalkParams& getServerParams();
    const rt::AvatorList& getAvatorList();

    bool isReady();
    async& talk(const rt::TalkParams& params, const std::string& text);
    async& stop();

    void wait();
    bool isFinished();
    rt::AudioData* syncBuffers();
    rt::AudioData* getBuffer();

private:
    rt::TalkClientSettings m_settings;
    rt::TalkClient m_client;

    rt::TalkParams m_server_params;
    rt::AvatorList m_avator_list;
    rt::AudioData m_buf_receiving;
    rt::AudioData m_buf_public;
    std::mutex m_mutex;
    async m_task_status;
    async m_task_talk;
    async m_task_stop;
    bool m_processing = false;
};


