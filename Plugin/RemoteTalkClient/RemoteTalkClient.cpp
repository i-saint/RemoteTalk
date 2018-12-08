#include "pch.h"
#include "RemoteTalkClient.h"


rtHTTPClient::rtHTTPClient(const char *server, uint16_t port)
    : m_settings(server, port)
    , m_client(m_settings)
{
}

void rtHTTPClient::release()
{
    delete this;
}

bool rtHTTPClient::talk(const rt::TalkParams& params, const char *text)
{
    m_buf_public.clear();
    m_buf_receiving.clear();

    m_client.talk(text, [this](const rt::AudioData& ad) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_buf_receiving += ad;
    });
    return false;
}

bool rtHTTPClient::stop()
{
    return m_client.stop();
}

bool rtHTTPClient::ready()
{
    return m_client.ready();
}

void rtHTTPClient::syncBuffer()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_buf_public = m_buf_receiving;
}

rt::AudioData* rtHTTPClient::getBuffer()
{
    return &m_buf_public;
}


rtExport rtHTTPClient* rtHTTPClientCreate(const char *server, int port)
{
    return new rtHTTPClient(server, port);
}

rtExport void rtHTTPClientRelease(rtHTTPClient *self)
{
    if (self)
        self->release();
}

rtExport bool rtHTTPClientTalk(rtHTTPClient *self, const rt::TalkParams *p, const char *text)
{
    return self->talk(*p, text);
}

rtExport bool rtHTTPClientStop(rtHTTPClient *self)
{
    return self->stop();
}

rtExport bool rtHTTPClientReady(rtHTTPClient *self)
{
    return self->ready();
}

rtExport bool rtHTTPClientIsFinished(rtHTTPClient *self)
{
    return true;
}

rtExport int rtHTTPClientConsumeAudioData(rtHTTPClient *self, rt::TalkSampleCallback cb)
{
    return 0;
}
