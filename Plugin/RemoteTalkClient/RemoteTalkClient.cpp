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

rtAsync<bool>& rtHTTPClient::updateServerStats()
{
    m_task_stats.task = std::async(std::launch::async, [this]() {
        if (m_client.stats(m_server_stats)) {
            return true;
        }
        else {
            m_server_stats.host = "Server Not Found";
            return false;
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

rtAsync<bool>& rtHTTPClient::talk(const rt::TalkParams& params, const std::string& text)
{
    m_buf_public.clear();
    m_buf_receiving.clear();

    m_task_talk.task = std::async(std::launch::async, [this, params, text]() {
        return m_client.talk(params, text, [this](const rt::AudioData& ad) {
            if (ad.getSampleLength() != 0) {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_buf_receiving += ad;
            }
        });
    });
    return m_task_talk;
}

rtAsync<bool>& rtHTTPClient::stop()
{
    m_task_stop.task = std::async(std::launch::async, [this]() {
        return m_client.stop();
    });
    return m_task_stop;
}

rtAsync<bool>& rtHTTPClient::exportWave(const std::string& path)
{
    m_task_export.wait();

    auto tmp_buf = std::make_shared<rt::AudioData>(m_buf_public);
    m_task_export.task = std::async(std::launch::async, [tmp_buf, path]() {
        return ExportWave(*tmp_buf, path.c_str());
    });
    return m_task_export;
}

rtAsync<bool>& rtHTTPClient::exportOgg(const std::string& path, const rt::OggSettings& settings)
{
    m_task_export.wait();

    auto tmp_buf = std::make_shared<rt::AudioData>(m_buf_public);
    m_task_export.task = std::async(std::launch::async, [tmp_buf, path, settings]() {
        return ExportOgg(*tmp_buf, path.c_str(), settings);
    });
    return m_task_export;
}

void rtHTTPClient::wait()
{
    m_task_stats.wait();
    m_task_talk.wait();
    m_task_stop.wait();
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



#pragma region rtAsync
rtExport bool rtAsyncIsValid(rtAsyncBase *self)
{
    if (!self)
        return false;
    return self->isValid();
}
rtExport bool rtAsyncIsFinished(rtAsyncBase *self)
{
    if (!self)
        return false;
    return self->isFinished();
}
rtExport void rtAsyncWait(rtAsyncBase *self)
{
    if (!self)
        return;
    self->wait();
}
rtExport bool rtAsyncGetBool(rtAsyncBase *self, bool *dst)
{
    if (auto b = dynamic_cast<rtAsync<bool>*>(self)) {
        if (b->isValid()) {
            *dst = b->get();
            return true;
        }
    }
    return false;
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


#pragma region rtTalkParamInfo
using rtTalkParamInfo = rt::TalkParamInfo;
using rtTalkParams = rt::TalkParams;

rtExport const char* rtTalkParamInfoGetName(rtTalkParamInfo *self)
{
    if (!self)
        return "";
    return self->name.c_str();
}
rtExport float rtTalkParamInfoGetValue(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->value;
}
rtExport float rtTalkParamInfoGetRangeMin(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->range_min;
}
rtExport float rtTalkParamInfoGetRangeMax(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->range_max;
}

rtExport uint32_t rtTalkParamsGetHash(rtTalkParams *self)
{
    if (!self)
        return 0;
    return self->hash();
}
#pragma endregion


#pragma region rtCastInfo
using rtCastInfo = rt::CastInfo;

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
    return (int)self->params.size();
}
rtExport rtTalkParamInfo* rtCastInfoGetParamInfo(rtCastInfo *self, int i)
{
    if (!self)
        return nullptr;
    return &self->params[i];
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

rtExport rtAsyncBase* rtHTTPClientUpdateServerStatus(rtHTTPClient *self)
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

rtExport rtAsyncBase* rtHTTPClientTalk(rtHTTPClient *self, const rt::TalkParams *p, const char *text)
{
    if (!self)
        return nullptr;
    return &self->talk(*p, text);
}

rtExport rtAsyncBase* rtHTTPClientStop(rtHTTPClient *self)
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

rtExport rtAsyncBase* rtHTTPClientExportWave(rtHTTPClient *self, const char *path)
{
    if (!self || !path)
        return nullptr;
    return &self->exportWave(path);
}
rtExport rtAsyncBase* rtHTTPClientExportOgg(rtHTTPClient *self, const char *path, const rtOggSettings *settings)
{
    if (!self || !path || !settings)
        return nullptr;
    return &self->exportOgg(path, *settings);
}
#pragma endregion
