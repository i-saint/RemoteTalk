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
    std::future<void> onTalk(const rt::TalkParams& params, const std::string& text, std::ostream& os) override;
    bool onStop() override;
    bool onListTalkers(std::string& result) override;
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
    rt::EnumerateTopWindows([](HWND hw) {
        ::SendMessageA(hw, WM_TIMER, 0, 0);
    });
}

std::future<void> rtvrTalkServer::onTalk(const rt::TalkParams& params, const std::string& text, std::ostream& os)
{
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }

    auto& dsound = rtvrDSoundHandler::getInstance();
    dsound.mute = params.mute;

    if (!rtGetTalkInterface_()->talk(params, text.c_str(), &sampleCallbackS, this))
        return {};

    return std::async(std::launch::async, [this, &os]() {
        std::vector<rt::AudioDataPtr> tmp;
        for (;;) {
            {
                std::unique_lock<std::mutex> lock(m_data_mutex);
                tmp = m_data_queue;
                m_data_queue.clear();
            }

            for (auto& ad : tmp) {
                ad->serialize(os);
            }

            if (!tmp.empty() && tmp.back()->data.empty())
                break;
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

bool rtvrTalkServer::onStop()
{
    return rtGetTalkInterface_()->stop();
}

bool rtvrTalkServer::onListTalkers(std::string& result)
{
    auto ifs = rtGetTalkInterface_();
    int n = ifs->getNumTalkers();
    for (int i = 0; i < n; ++i) {
        rt::TalkerInfo ti;
        ifs->getTalkerInfo(i, &ti);
        char buf[256];
        sprintf(buf, "%d: %s\n", ti.id, ti.name);
        result += buf;

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
