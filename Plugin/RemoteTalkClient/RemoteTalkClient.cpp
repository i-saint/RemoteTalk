#include "pch.h"
#include "RemoteTalkClient.h"


rtHTTPClient::rtHTTPClient(const char *server, uint16_t port)
    : m_settings(server, port)
    , m_client(m_settings)
{
}

rtHTTPClient::~rtHTTPClient()
{
    wait();
}

void rtHTTPClient::release()
{
    delete this;
}

void rtHTTPClient::updateParams()
{
    m_task_status = std::async(std::launch::async, [this]() {
        m_client.getParams(m_server_params, m_avator_list);
    });
}

const rt::TalkParams& rtHTTPClient::getServerParams()
{
    return m_server_params;
}

const rt::AvatorList& rtHTTPClient::getAvatorList()
{
    return m_avator_list;
}

bool rtHTTPClient::isReady()
{
    return m_client.ready();
}

bool rtHTTPClient::talk(const rt::TalkParams& params, const std::string& text)
{
    m_buf_public.clear();
    m_buf_receiving.clear();

    m_task_talk = std::async(std::launch::async, [this, params, text]() {
        m_client.talk(params, text, [this](const rt::AudioData& ad) {
            if (ad.getSampleLength() != 0) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_buf_receiving += ad;
            }
        });
    });
    return true;
}

bool rtHTTPClient::stop()
{
    return m_client.stop();
}

void rtHTTPClient::wait()
{
    if (m_task_talk.valid())
        m_task_talk.wait();
}

bool rtHTTPClient::isFinished()
{
    return !m_task_talk.valid() || m_task_talk.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

rt::AudioData* rtHTTPClient::syncBuffers()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_buf_public.data.size() != m_buf_receiving.data.size())
        m_buf_public = m_buf_receiving;
    return getBuffer();
}

rt::AudioData* rtHTTPClient::getBuffer()
{
    return &m_buf_public;
}



#pragma region rtAudioData

using rtAudioData = rt::AudioData;
using rtAudioFormat = rt::AudioFormat;

rtExport rtAudioData* rtAudioDataCreate()
{
    return new rtAudioData();
}

rtExport void rtAudioDataRelease(rtAudioData *self)
{
    delete self;
}

rtExport void rtAudioDataAppend(rtAudioData *self, rtAudioData *v)
{
    if (!self)
        return;
    *self += *v;
}

rtExport rtAudioFormat rtAudioDataGetFormat(rtAudioData *self)
{
    if (!self)
        return rtAudioFormat::Unknown;
    return self->format;
}

rtExport int rtAudioDataGetChannels(rtAudioData *self)
{
    if (!self)
        return 0;
    return self->channels;
}

rtExport int rtAudioDataGetFrequency(rtAudioData *self)
{
    if (!self)
        return 0;
    return self->frequency;
}

rtExport int rtAudioDataGetSampleLength(rtAudioData *self)
{
    if (!self)
        return 0;
    return (int)self->getSampleLength();
}

rtExport bool rtAudioDataReadSamplesFloat(rtAudioData *self, float *dst, int beg, int end)
{
    if (!self)
        return false;
    return self->convertSamplesToFloat(dst, beg, end);
}

rtExport bool rtAudioDataExportAsWave(rtAudioData *self, const char *path)
{
    if (!self)
        return false;
    return self->exportAsWave(path);
}
#pragma endregion


#pragma region rtHTTPClient
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
    if (!self)
        return false;
    return self->talk(*p, text);
}

rtExport bool rtHTTPClientStop(rtHTTPClient *self)
{
    if (!self)
        return false;
    return self->stop();
}

rtExport bool rtHTTPClientIsReady(rtHTTPClient *self)
{
    if (!self)
        return false;
    return self->isReady();
}

rtExport bool rtHTTPClientIsFinished(rtHTTPClient *self)
{
    if (!self)
        return true;
    return self->isFinished();
}

rtExport rtAudioData* rtHTTPClientSyncBuffers(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return self->syncBuffers();
}

rtExport rtAudioData* rtHTTPClientGetBuffer(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return self->getBuffer();
}
#pragma endregion
