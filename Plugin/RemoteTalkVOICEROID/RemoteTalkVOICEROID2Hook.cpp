#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "RemoteTalkVOICEROID2Controller.h"
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
    bool onTalk(TalkMessage& mes) override;
    bool onStop(StopMessage& mes) override;
    bool onListTalkers(ListTalkersMessage& mes) override;
    bool ready() override;
#ifdef rtDebug
    void onDebug() override;
#endif

    static void sampleCallbackS(const rt::TalkSample *data, void *userdata);
    void sampleCallback(const rt::TalkSample *data);

private:
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;
};


rtvr2TalkInterface* (*rtGetTalkInterface_)();
static bool rtvr2LoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkVOICEROID2Managed.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, "rtGetTalkInterface");
    return rtGetTalkInterface_;
}

static void SendTimerMessage()
{
    rt::EnumerateTopWindows([](HWND hw) {
        ::SendMessageA(hw, WM_TIMER, 0, 0);
    });
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
    SendTimerMessage();
}

bool rtvrTalkServer::onTalk(TalkMessage& mes)
{
    if (!rtGetTalkInterface_()->prepareUI()) {
        // UI needs refresh. wait next message.
        SendTimerMessage();
        return false;
    }

    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }

    if (mes.params.flags.fields.mute) {
        auto& dsound = rtvrDSoundHandler::getInstance();
        dsound.mute = mes.params.mute;
    }

    if (!rtGetTalkInterface_()->talk(mes.params, mes.text.c_str(), &sampleCallbackS, this))
        return true;

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
    return true;
}

bool rtvrTalkServer::onStop(StopMessage& mes)
{
    return rtGetTalkInterface_()->stop();
}

bool rtvrTalkServer::onListTalkers(ListTalkersMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    int n = ifs->getNumTalkers();
    for (int i = 0; i < n; ++i) {
        rt::TalkerInfo ti;
        ifs->getTalkerInfo(i, &ti);
        char buf[256];
        sprintf(buf, "%d: %s\n", ti.id, ti.name);
        mes.result += buf;

    }
    return false;
}

bool rtvrTalkServer::ready()
{
    return rtGetTalkInterface_()->ready();
}

#ifdef rtDebug
void rtvrTalkServer::onDebug()
{
    return rtGetTalkInterface_()->onDebug();
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
        if (rtvr2LoadManagedModule()) {
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
