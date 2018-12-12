#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "RemoteTalkVOICEROIDCommon.h"
#include "rtvrDSound.h"



class rtvrWindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    rtDefSingleton(rtvrWindowMessageHandler);
    void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
};

class rtvrTalkServer : public rt::TalkServer
{
using super = rt::TalkServer;
public:
    rtDefSingleton(rtvrTalkServer);
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


rtvr2ITalkInterface* (*rtGetTalkInterface_)();
static bool rtcvLoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkVOICEROID2Managed.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, rtInterfaceFuncName);
    return rtGetTalkInterface_;
}

static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}


void rtvrWindowMessageHandler::afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    auto& server = rtvrTalkServer::getInstance();
    server.start();
    server.processMessages();
}

void rtvrTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool rtvrTalkServer::ready()
{
    return rtGetTalkInterface_()->ready();
}

rtvrTalkServer::Status rtvrTalkServer::onStats(StatsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        RequestUpdate();
        return Status::Pending;
    }

    auto& stats = mes.stats;
    if (!ifs->getParams(stats.params)) {
        RequestUpdate();
        return Status::Pending;
    }
    {
        int n = ifs->getNumCasts();
        for (int i = 0; i < n; ++i) {
            rt::CastInfo ci;
            ifs->getCastInfo(i, &ci);

            rt::CastInfoImpl cii;
            cii.fromCastInfo(ci);
            stats.casts.push_back(std::move(cii));
        }
    }
    stats.host = ifs->getClientName();
    stats.plugin_version = ifs->getPluginVersion();
    stats.protocol_version = ifs->getProtocolVersion();
    return Status::Succeeded;
}

rtvrTalkServer::Status rtvrTalkServer::onTalk(TalkMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        RequestUpdate();
        return Status::Pending;
    }
    if (ifs->stop()) {
        // need to wait until next message if stop() succeeded.
        RequestUpdate();
        return Status::Pending;
    }
    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());

    rtvrDSoundHandler::getInstance().mute = mes.params.mute;
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    if (!ifs->talk(&sampleCallbackS, this))
        return Status::Failed;

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

rtvrTalkServer::Status rtvrTalkServer::onStop(StopMessage& mes)
{
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        RequestUpdate();
        return Status::Pending;
    }
    return ifs->stop() ? Status::Succeeded : Status::Failed;
}

#ifdef rtDebug
rtvrTalkServer::Status rtvrTalkServer::onDebug(DebugMessage& mes)
{
    return rtGetTalkInterface_()->onDebug() ? Status::Succeeded : Status::Failed;
}
#endif


void rtvrTalkServer::sampleCallbackS(const rt::TalkSample *data, void *userdata)
{
    auto _this = (rtvrTalkServer*)userdata;
    _this->sampleCallback(data);
}

void rtvrTalkServer::sampleCallback(const rt::TalkSample *data)
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
            rt::AddWindowMessageHandler(&rtvrWindowMessageHandler::getInstance());

            auto& dsound = rtvrDSoundHandler::getInstance();
            rt::AddDSoundHandler(&dsound);
            dsound.onPlay = []() { rtGetTalkInterface_()->onPlay(); };
            dsound.onStop = []() { rtGetTalkInterface_()->onStop(); };
            dsound.onUpdate = [](const rt::AudioData& ad) { rtGetTalkInterface_()->onUpdateBuffer(ad); };
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
    auto& dsound = rtvrDSoundHandler::getInstance();
    dsound.onPlay = {};
    dsound.onStop = {};
    dsound.onUpdate = {};
}
