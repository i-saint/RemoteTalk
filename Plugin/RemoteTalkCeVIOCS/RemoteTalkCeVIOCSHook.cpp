#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "RemoteTalkCeVIOCSCommon.h"


class rtcvWaveOutHandler : public rt::WaveOutHandlerBase
{
public:
    rtDefSingleton(rtcvWaveOutHandler);
};


class rtcvTalkServer : public rt::TalkServer
{
using super = rt::TalkServer;
public:
    rtDefSingleton(rtcvTalkServer);
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
            rt::AddWaveOutHandler(&rtcvWaveOutHandler::getInstance());
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}

rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return rtGetTalkInterface_ ? rtGetTalkInterface_() : nullptr;
}

rtExport void rtOnManagedModuleUnload()
{
    auto& waveout = rtcvWaveOutHandler::getInstance();
}
