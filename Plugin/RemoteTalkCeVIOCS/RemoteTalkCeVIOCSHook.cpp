#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"


class rtcvWaveOutHandler : public rt::WaveOutHandlerBase
{
public:
    rtDefSingleton(rtcvWaveOutHandler);
};

class rtcvWindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    rtDefSingleton(rtcvWindowMessageHandler);
    void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
};

class rtcvTalkServer : public rt::TalkServer
{
using super = rt::TalkServer;
public:
    rtDefSingleton(rtcvTalkServer);
    void addMessage(MessagePtr mes) override;
    bool onStats(StatsMessage& mes) override;
    bool onTalk(TalkMessage& mes) override;
    bool onStop(StopMessage& mes) override;
    bool ready() override;
#ifdef rtDebug
    bool onDebug(DebugMessage& mes) override;
#endif

    static void sampleCallbackS(const rt::TalkSample *data, void *userdata);
    void sampleCallback(const rt::TalkSample *data);

private:
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;
};



rt::TalkInterface* (*rtGetTalkInterface_)();
static bool rtcvLoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkCeVIOCSManaged.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, "rtGetTalkInterface");
    return rtGetTalkInterface_;
}

static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}

void rtcvWindowMessageHandler::afterGetMessageW(LPMSG & lpMsg, HWND & hWnd, UINT & wMsgFilterMin, UINT & wMsgFilterMax, BOOL & ret)
{
    auto& server = rtcvTalkServer::getInstance();
    server.start();
    server.processMessages();
}


void rtcvTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool rtcvTalkServer::onStats(StatsMessage & mes)
{
    return false;
}

bool rtcvTalkServer::onTalk(TalkMessage & mes)
{
    return false;
}

bool rtcvTalkServer::onStop(StopMessage & mes)
{
    return false;
}

bool rtcvTalkServer::ready()
{
    return false;
}

#ifdef rtDebug
bool rtcvTalkServer::onDebug(DebugMessage& mes)
{
    return false;
}
#endif



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (rtcvLoadManagedModule()) {
            rt::AddWindowMessageHandler(&rtcvWindowMessageHandler::getInstance());
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
