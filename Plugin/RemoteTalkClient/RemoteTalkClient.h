#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

class rtHTTPClient
{
public:
    rtHTTPClient(const char *server, uint16_t port);
    void release();

    bool talk(const rt::TalkParams& params, const char *text);
    bool stop();
    bool ready();

    rt::AudioData* syncBuffers();
    rt::AudioData* getBuffer();

private:
    rt::TalkClientSettings m_settings;
    rt::TalkClient m_client;
    rt::AudioData m_buf_receiving;
    rt::AudioData m_buf_public;
    std::mutex m_mutex;
    bool m_processing = false;
};


