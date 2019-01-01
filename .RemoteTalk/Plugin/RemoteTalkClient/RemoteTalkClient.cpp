#include "pch.h"
#include "RemoteTalkClient.h"


rtHTTPClient::rtHTTPClient()
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

void rtHTTPClient::reset(const char *address, uint16_t port)
{
    m_settings = {address, port};
    m_client = rt::TalkClient(m_settings);
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

rtAsync<bool>& rtHTTPClient::play(const rt::TalkParams& params, const std::string& text)
{
    m_buf_public.clear();
    m_buf_receiving.clear();

    m_task_talk.task = std::async(std::launch::async, [this, params, text]() {
        return m_client.play(params, text, [this](const rt::AudioData& ad) {
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
    m_task_stats.wait(30000);
    m_task_talk.wait(30000);
    m_task_stop.wait(30000);
    m_task_export.wait(30000);
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
rtAPI const char* rtGetVersion()
{
    return rtPluginVersionStr;
}
#pragma endregion


#pragma region rtAsync
rtAPI bool rtAsyncIsValid(rtAsyncBase *self)
{
    if (!self)
        return false;
    return self->isValid();
}
rtAPI bool rtAsyncIsFinished(rtAsyncBase *self)
{
    if (!self)
        return false;
    return self->isFinished();
}
rtAPI bool rtAsyncWait(rtAsyncBase *self, int timeout_ms)
{
    if (!self)
        return false;
    return self->wait(timeout_ms);
}
rtAPI bool rtAsyncGetBool(rtAsyncBase *self, bool *dst)
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

rtAPI rtAudioData* rtAudioDataCreate()
{
    return new rtAudioData();
}

rtAPI void rtAudioDataRelease(rtAudioData *self)
{
    delete self;
}

rtAPI void rtAudioDataAppend(rtAudioData *self, rtAudioData *v)
{
    if (!self)
        return;
    *self += *v;
}

rtAPI rtAudioFormat rtAudioDataGetFormat(rtAudioData *self)
{
    if (!self)
        return rtAudioFormat::Unknown;
    return self->format;
}

rtAPI int rtAudioDataGetChannels(rtAudioData *self)
{
    if (!self)
        return 0;
    return self->channels;
}

rtAPI int rtAudioDataGetFrequency(rtAudioData *self)
{
    if (!self)
        return 0;
    return self->frequency;
}

rtAPI int rtAudioDataGetSampleLength(rtAudioData *self)
{
    if (!self)
        return 0;
    return (int)self->getSampleLength();
}

rtAPI int rtAudioDataReadSamples(rtAudioData *self, float *dst, int pos, int len)
{
    if (!self)
        return 0;
    return self->toFloat(dst, pos, len);
}

rtAPI double rtAudioDataReample(rtAudioData *self, float *dst, int frequency, int channels, int length, double pos)
{
    if (!self)
        return 0;
    return self->resampleFloat(dst, frequency, channels, length, pos);
}

rtAPI void rtAudioDataClearSample(float *dst, int len)
{
    for (int i = 0; i < len; ++i)
        dst[i] = 0.0f;
}

rtAPI bool rtAudioDataExportWave(rtAudioData *self, const char *path)
{
    if (!self || !path)
        return false;
    return rt::ExportWave(*self, path);
}

rtAPI bool rtAudioDataExportOgg(rtAudioData *self, const char *path, const rtOggSettings *settings)
{
    if (!self || !path || !settings)
        return false;
    return rt::ExportOgg(*self, path, *settings);
}
#pragma endregion


#pragma region rtTalkParamInfo
using rtTalkParamInfo = rt::TalkParamInfo;
using rtTalkParams = rt::TalkParams;

rtAPI const char* rtTalkParamInfoGetName(rtTalkParamInfo *self)
{
    if (!self)
        return "";
    return self->name.c_str();
}
rtAPI float rtTalkParamInfoGetValue(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->value;
}
rtAPI float rtTalkParamInfoGetRangeMin(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->range_min;
}
rtAPI float rtTalkParamInfoGetRangeMax(rtTalkParamInfo *self)
{
    if (!self)
        return 0;
    return self->range_max;
}

rtAPI uint32_t rtTalkParamsGetHash(rtTalkParams *self)
{
    if (!self)
        return 0;
    return self->hash();
}
#pragma endregion


#pragma region rtCastInfo
using rtCastInfo = rt::CastInfo;

rtAPI int rtCastInfoGetID(rtCastInfo *self)
{
    if (!self)
        return 0;
    return self->id;
}
rtAPI const char* rtCastInfoGetName(rtCastInfo *self)
{
    if (!self)
        return "";
    return self->name.c_str();
}
rtAPI int rtCastInfoGetNumParams(rtCastInfo *self)
{
    if (!self)
        return 0;
    return (int)self->params.size();
}
rtAPI rtTalkParamInfo* rtCastInfoGetParamInfo(rtCastInfo *self, int i)
{
    if (!self)
        return nullptr;
    return &self->params[i];
}

#pragma endregion


#pragma region rtHTTPClient
using rtTalkParams = rt::TalkParams;

rtAPI rtHTTPClient* rtHTTPClientCreate()
{
    return new rtHTTPClient();
}

rtAPI void rtHTTPClientRelease(rtHTTPClient *self)
{
    if (self)
        self->release();
}

rtAPI void rtHTTPClientSetup(rtHTTPClient *self, const char *address, int port)
{
    if (self)
        self->reset(address, port);
}

rtAPI rtAsyncBase* rtHTTPClientUpdateServerStatus(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->updateServerStats();
}

rtAPI const char* rtHTTPClientGetServerHostApp(rtHTTPClient *self)
{
    if (!self)
        return "";
    return self->getServerStats().host.c_str();
}

rtAPI void rtHTTPClientGetServerParams(rtHTTPClient *self, rtTalkParams *st)
{
    if (!self)
        return;
    *st = self->getServerStats().params;
}

rtAPI int rtHTTPClientGetNumCasts(rtHTTPClient *self)
{
    if (!self)
        return 0;
    return (int)self->getServerStats().casts.size();
}

rtAPI const rtCastInfo* rtHTTPClientGetCast(rtHTTPClient *self, int i)
{
    if (!self)
        return nullptr;
    return &self->getServerStats().casts[i];
}

rtAPI rtAsyncBase* rtHTTPClientTalk(rtHTTPClient *self, const rt::TalkParams *p, const char *text)
{
    if (!self)
        return nullptr;
    return &self->play(*p, text);
}

rtAPI rtAsyncBase* rtHTTPClientStop(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->stop();
}

rtAPI bool rtHTTPClientIsReady(rtHTTPClient *self)
{
    if (!self)
        return false;
    return self->isReady();
}

rtAPI const rtAudioData* rtHTTPClientSyncBuffers(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->syncBuffers();
}

rtAPI const rtAudioData* rtHTTPClientGetBuffer(rtHTTPClient *self)
{
    if (!self)
        return nullptr;
    return &self->getBuffer();
}

rtAPI rtAsyncBase* rtHTTPClientExportWave(rtHTTPClient *self, const char *path)
{
    if (!self || !path)
        return nullptr;
    return &self->exportWave(path);
}
rtAPI rtAsyncBase* rtHTTPClientExportOgg(rtHTTPClient *self, const char *path, const rtOggSettings *settings)
{
    if (!self || !path || !settings)
        return nullptr;
    return &self->exportOgg(path, *settings);
}
#pragma endregion
