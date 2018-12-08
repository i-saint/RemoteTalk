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

    m_processing = true;
    bool ret = m_client.talk(params, text, [this](const rt::AudioData& ad) {
        if (ad.getSampleLength() == 0) {
            m_processing = false;
        }
        else {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_buf_receiving += ad;
        }
    });
    if (!ret)
        m_processing = false;
    return ret;
}

bool rtHTTPClient::stop()
{
    return m_client.stop();
}

bool rtHTTPClient::ready()
{
    return m_client.ready();
}

rt::AudioData* rtHTTPClient::syncBuffers()
{
    std::unique_lock<std::mutex> lock(m_mutex);
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
    *self += *v;
}

rtExport rtAudioFormat rtAudioDataGetFormat(rtAudioData *self)
{
    return self->format;
}

rtExport int rtAudioDataGetChannels(rtAudioData *self)
{
    return self->channels;
}

rtExport int rtAudioDataGetFrequency(rtAudioData *self)
{
    return self->frequency;
}

rtExport int rtAudioDataGetSampleLength(rtAudioData *self)
{
    return (int)self->getSampleLength();
}

rtExport bool rtAudioDataReadSamplesFloat(rtAudioData *self, float *dst, int beg, int end)
{
    return self->convertSamplesToFloat(dst, beg, end);
}

rtExport bool rtAudioDataExportAsWave(rtAudioData *self, const char *path)
{
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
    return self;
}

rtExport rtAudioData* rtHTTPClientSyncBuffers(rtHTTPClient *self)
{
    return self->syncBuffers();
}

rtExport rtAudioData* rtHTTPClientGetBuffer(rtHTTPClient *self)
{
    return self->getBuffer();
}
#pragma endregion
