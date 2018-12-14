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

rtHTTPClient::async& rtHTTPClient::updateServerStats()
{
    m_task_stats = std::async(std::launch::async, [this]() {
        if (!m_client.stats(m_server_stats)) {
            m_server_stats.host = "Server Not Found";
        }
    });
    return m_task_stats;
}

const rt::TalkServerStats& rtHTTPClient::getServerStats() const
{
    return m_server_stats;
}

bool rtHTTPClient::isReady()
{
    return m_client.ready();
}

rtHTTPClient::async& rtHTTPClient::talk(const rt::TalkParams& params, const std::string& text)
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
    return m_task_talk;
}

rtHTTPClient::async& rtHTTPClient::stop()
{
    m_task_stop = std::async(std::launch::async, [this]() {
        m_client.stop();
    });
    return m_task_stop;
}

rtHTTPClient::async& rtHTTPClient::exportWave(const std::string& path)
{
    if (m_task_export.valid())
        m_task_export.wait();

    auto tmp_buf = std::make_shared<rt::AudioData>(m_buf_public);
    m_task_export = std::async(std::launch::async, [tmp_buf, path]() {
        ExportWave(*tmp_buf, path.c_str());
    });
    return m_task_export;
}

rtHTTPClient::async& rtHTTPClient::exportOgg(const std::string& path, const rt::OggSettings& settings)
{
    if (m_task_export.valid())
        m_task_export.wait();

    auto tmp_buf = std::make_shared<rt::AudioData>(m_buf_public);
    m_task_export = std::async(std::launch::async, [tmp_buf, path, settings]() {
        ExportOgg(*tmp_buf, path.c_str(), settings);
    });
    return m_task_export;
}

void rtHTTPClient::wait()
{
    if (m_task_stats.valid())
        m_task_stats.wait();
    if (m_task_talk.valid())
        m_task_talk.wait();
    if (m_task_stop.valid())
        m_task_stop.wait();
    if (m_task_export.valid())
        m_task_export.wait();
}

const rt::AudioData& rtHTTPClient::syncBuffers()
{
    if (m_buf_receiving.data.empty())
        return m_buf_public;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_buf_receiving.data.empty()) {
            m_buf_public += m_buf_receiving;
            m_buf_receiving.clear();
        }
        return m_buf_public;
    }
}

const rt::AudioData& rtHTTPClient::getBuffer()
{
    return m_buf_public;
}



#pragma region rtAudioData
using rtAsync = std::future<void>;

rtExport bool rtAsyncIsFinished(rtAsync *self)
{
    if (!self)
        return true;
    return self->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
}

rtExport void rtAsyncWait(rtAsync *self)
{
    if (!self)
        return;
    self->wait();
}
#pragma endregion



#pragma region rtAudioData
using rtAudioData = rt::AudioData;
using rtAudioFormat = rt::AudioFormat;
using rtOggSettings = rt::OggSettings;

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

rtExport int rtAudioDataReadSamples(rtAudioData *self, float *dst, int pos, int len)
{
    if (!self)
        return 0;
    return self->toFloat(dst, pos, len);
}

rtExport double rtAudioDataReample(rtAudioData *self, float *dst, int frequency, int channels, int length, double pos)
{
    if (!self)
        return 0;
    return self->resampleFloat(dst, frequency, channels, length, pos);
}

rtExport void rtAudioDataClearSample(float *dst, int len)
{
    for (int i = 0; i < len; ++i)
        dst[i] = 0.0f;
}

rtExport bool rtAudioDataExportWave(rtAudioData *self, const char *path)
{
    if (!self || !path)
        return false;
    return rt::ExportWave(*self, path);
}

rtExport bool rtAudioDataExportOgg(rtAudioData *self, const char *path, const rtOggSettings *settings)
{
    if (!self || !path || !settings)
        return false;
    return rt::ExportOgg(*self, path, *settings);
}
#pragma endregion


#pragma region rtCastInfo
using rtCastInfo = rt::CastInfoImpl;

rtExport int rtCastInfoGetID(rtCastInfo *self)
{
    if (!self)
        return 0;
    return self->id;
}
rtExport const char* rtCastInfoGetName(rtCastInfo *self)
{
    if (!self)
        return "";
    return self->name.c_str();
}
rtExport int rtCastInfoGetNumParams(rtCastInfo *self)
{
    if (!self)
        return 0;
    return (int)self->param_names.size();
}
rtExport const char* rtCastInfoGetParamName(rtCastInfo *self, int i)
{
    if (!self)
        return "";
    return self->param_names[i].c_str();
}

#pragma endregion


#pragma region rtHTTPClient
using rtTalkParams = rt::TalkParams;

rtExport rtHTTPClient* rtHTTPClientCreate(const char *server, int port)
{
    return new rtHTTPClient(server, port);
}

rtExport void rtHTTPClientRelease(rtHTTPClient *self)
{
    if (self)
        self->release();
}

rtExport rtAsync* rtHTTPClientUpdateServerStatus(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->updateServerStats();
}

rtExport const char* rtHTTPClientGetServerHostApp(rtHTTPClient *self)
{
    if (!self)
        return "";
    return self->getServerStats().host.c_str();
}

rtExport void rtHTTPClientGetServerParams(rtHTTPClient *self, rtTalkParams *st)
{
    if (!self)
        return;
    *st = self->getServerStats().params;
}

rtExport int rtHTTPClientGetNumCasts(rtHTTPClient *self)
{
    if (!self)
        return 0;
    return (int)self->getServerStats().casts.size();
}

rtExport const rtCastInfo* rtHTTPClientGetCast(rtHTTPClient *self, int i)
{
    if (!self)
        return nullptr;
    return &self->getServerStats().casts[i];
}

rtExport rtAsync* rtHTTPClientTalk(rtHTTPClient *self, const rt::TalkParams *p, const char *text)
{
    if (!self)
        return nullptr;
    return &self->talk(*p, text);
}

rtExport rtAsync* rtHTTPClientStop(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->stop();
}

rtExport bool rtHTTPClientIsReady(rtHTTPClient *self)
{
    if (!self)
        return false;
    return self->isReady();
}

rtExport const rtAudioData* rtHTTPClientSyncBuffers(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->syncBuffers();
}

rtExport const rtAudioData* rtHTTPClientGetBuffer(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->getBuffer();
}

rtExport rtAsync* rtHTTPClientExportWave(rtHTTPClient *self, const char *path)
{
    if (!self || !path)
        return nullptr;
    return &self->exportWave(path);
}
rtExport rtAsync* rtHTTPClientExportOgg(rtHTTPClient *self, const char *path, const rtOggSettings *settings)
{
    if (!self || !path || !settings)
        return nullptr;
    return &self->exportOgg(path, *settings);
}
#pragma endregion
