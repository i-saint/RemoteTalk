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
    bool onTalk(TalkMessage& mes) override;
    bool onStop(StopMessage& mes) override;
    bool onGetParams(GetParamsMessage& mes) override;
    bool ready() override;
#ifdef rtDebug
    bool onDebug() override;
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
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        return false;
    }
    if (ifs->stop()) {
        // need to wait until next message if stop() succeeded.
        return false;
    }
    ifs->setParams(mes.params);
    ifs->setText(mes.text.c_str());

    if (mes.params.flags.mute) {
        rtvrDSoundHandler::getInstance().mute = mes.params.mute;
    }
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }
    if (!ifs->talk(&sampleCallbackS, this))
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
    auto *ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI()) {
        // UI needs refresh. wait next message.
        SendTimerMessage();
        return false;
    }
    return ifs->stop();
}

bool rtvrTalkServer::onGetParams(GetParamsMessage& mes)
{
    auto ifs = rtGetTalkInterface_();
    if (!ifs->prepareUI())
        return false;
    if (!ifs->getParams(mes.params))
        return false;
    {
        int n = ifs->getNumAvators();
        for (int i = 0; i < n; ++i) {
            rt::AvatorInfo ti;
            ifs->getAvatorInfo(i, &ti);
            mes.avators.push_back({ ti.id, ti.name });
        }
    }
    return true;
}

bool rtvrTalkServer::ready()
{
    return rtGetTalkInterface_()->ready();
}

#ifdef rtDebug
bool rtvrTalkServer::onDebug()
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
