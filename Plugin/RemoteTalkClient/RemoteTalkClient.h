#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

class rtHTTPClient
{
public:
    rtHTTPClient(const char *server, uint16_t port);
    ~rtHTTPClient();
    void release();

    void updateParams();
    const rt::TalkParams& getServerParams();
    const rt::AvatorList& getAvatorList();

    bool isReady();
    bool talk(const rt::TalkParams& params, const std::string& text);
    bool stop();

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
    std::future<void> m_task_status;
    std::future<void> m_task_talk;
    bool m_processing = false;
};


