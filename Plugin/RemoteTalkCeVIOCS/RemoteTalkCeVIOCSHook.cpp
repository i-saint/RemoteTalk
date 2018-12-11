#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "RemoteTalkCeVIOCSCommon.h"


class rtcvWindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    rtDefSingleton(rtcvWindowMessageHandler);
    void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
};

class rtcvWaveOutHandler : public rt::WaveOutHandlerBase
{
public:
    rtDefSingleton(rtcvWaveOutHandler);

    bool mute = false;
    std::function<void()> onPlay, onStop;
    std::function<void(rt::AudioData&)> onUpdate;

    void clearCallbacks();

protected:
    void afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) override;
    void beforeWaveOutClose(HWAVEOUT& hwo) override;
    //void afterWaveOutPrepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) override;
    //void beforeWaveOutUnprepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) override;
    void beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) override;
    //void afterWaveOutPause(HWAVEOUT& hwo, MMRESULT& ret) override;
    //void beforeWaveOutRestart(HWAVEOUT& hwo) override;
    void beforeWaveOutReset(HWAVEOUT& hwo) override;

private:
    struct Record
    {
        WAVEFORMATEX wave_format;
        rt::AudioData data;
        bool is_opened = false;
        bool is_playing = false;
    };
    std::map<HWAVEOUT, Record> m_records;
};


class rtcvTalkServer : public rt::TalkServer
{
using super = rt::TalkServer;
public:
    rtDefSingleton(rtcvTalkServer);
    rtcvTalkServer();
    void addMessage(MessagePtr mes) override;

    bool ready() override;
    Status onStats(StatsMessage& mes) override;
    Status onTalk(TalkMessage& mes) override;
    Status onStop(StopMessage& mes) override;
#ifdef rtDebug
    Status onDebug(DebugMessage& mes) override;
#endif

    static void sampleCallbackS(const rt::TalkSample *data, void *userdata);
    void sampleCallback(const rt::TalkSample *data);

private:
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;
};



rtcvITalkInterface* (*rtGetTalkInterface_)();
static bool rtcvLoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkCeVIOCSManaged.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, rtInterfaceFuncName);
    return rtGetTalkInterface_;
}



void rtcvWindowMessageHandler::afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    auto& server = rtcvTalkServer::getInstance();
    server.start();
}


void rtcvWaveOutHandler::clearCallbacks()
{
    onPlay = {};
    onStop = {};
    onUpdate = {};
}

void rtcvWaveOutHandler::afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret)
{
    if (FAILED(ret))
        return;
    auto& rec = m_records[*phwo];
    rec.is_opened = true;
    rec.wave_format = *pwfx;

    rec.data.frequency = rec.wave_format.nSamplesPerSec;
    rec.data.channels = rec.wave_format.nChannels;
    switch (rec.wave_format.wBitsPerSample) {
    case 8: rec.data.format = rt::AudioFormat::U8; break;
    case 16: rec.data.format = rt::AudioFormat::S16; break;
    case 24: rec.data.format = rt::AudioFormat::S24; break;
    case 32: rec.data.format = rt::AudioFormat::S32; break;
    }
}

void rtcvWaveOutHandler::beforeWaveOutClose(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing) {
        rec.is_playing = false;
        if (onStop)
            onStop();
    }
    rec.is_opened = false;
}

void rtcvWaveOutHandler::beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    rec.data.data.assign(pwh->lpData, pwh->lpData + pwh->dwBufferLength);
    if (!rec.is_playing) {
        rec.is_playing = true;
        if (onPlay)
            onPlay();
    }
    if (onUpdate)
        onUpdate(rec.data);
}

void rtcvWaveOutHandler::beforeWaveOutReset(HWAVEOUT& hwo)
{
    auto it = m_records.find(hwo);
    if (it == m_records.end())
        return;

    auto& rec = it->second;
    if (rec.is_playing) {
        rec.is_playing = false;
        if (onStop)
            onStop();
    }
}



rtcvTalkServer::rtcvTalkServer()
{
    m_settings.port = 8082;
}

void rtcvTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
    processMessages();
}

bool rtcvTalkServer::ready()
{
    return false;
}

rtcvTalkServer::Status rtcvTalkServer::onStats(StatsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();

    auto& stats = mes.stats;
    ifs->getParams(stats.params);
    {
        int n = ifs->getNumCasts();
        for (int i = 0; i < n; ++i) {
            rt::CastInfo ti;
            ifs->getCastInfo(i, &ti);
            stats.casts.push_back({ ti.id, ti.name });
        }
    }
    stats.host = ifs->getClientName();
    stats.plugin_version = ifs->getPluginVersion();
    stats.protocol_version = ifs->getProtocolVersion();
    return Status::Succeeded;
}

rtcvTalkServer::Status rtcvTalkServer::onTalk(TalkMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());
    if (!ifs->talk(&sampleCallbackS, this))
        Status::Failed;

    mes.task = std::async(std::launch::async, [this, &mes]() {
        std::vector<rt::AudioDataPtr> tmp;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(m_data_mutex);
                tmp = m_data_queue;
                m_data_queue.clear();
            }

            for (auto& ad : tmp) {
                ad->serialize(*mes.respond_stream);
            }

            if (!tmp.empty() && tmp.back()->data.empty())
                break;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    return Status::Succeeded;
}

rtcvTalkServer::Status rtcvTalkServer::onStop(StopMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    return ifs->stop() ? Status::Succeeded : Status::Failed;
}

#ifdef rtDebug
rtcvTalkServer::Status rtcvTalkServer::onDebug(DebugMessage& mes)
{
    return rtGetTalkInterface_()->onDebug() ? Status::Succeeded : Status::Failed;
}
#endif

void rtcvTalkServer::sampleCallbackS(const rt::TalkSample *data, void *userdata)
{
    auto _this = (rtcvTalkServer*)userdata;
    _this->sampleCallback(data);
}

void rtcvTalkServer::sampleCallback(const rt::TalkSample *data)
{
    auto tmp = std::make_shared<rt::AudioData>();
    rt::ToAudioData(*tmp, *data);
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (rtcvLoadManagedModule()) {
            rt::AddWindowMessageHandler(&rtcvWindowMessageHandler::getInstance());

            auto& wo = rtcvWaveOutHandler::getInstance();
            rt::AddWaveOutHandler(&wo);
            wo.onPlay = []() { rtGetTalkInterface_()->onPlay(); };
            wo.onStop = []() { rtGetTalkInterface_()->onStop(); };
            wo.onUpdate = [](rt::AudioData& ad) { rtGetTalkInterface_()->onUpdateBuffer(ad); };
        }
    }
    return TRUE;
}

rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return rtGetTalkInterface_ ? rtGetTalkInterface_() : nullptr;
}

rtExport void rtOnManagedModuleUnload()
{
    rtcvWaveOutHandler::getInstance().clearCallbacks();
}
