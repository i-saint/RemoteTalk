#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

#define _ATL_APARTMENT_THREADED
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <sapi.h>
#include <sphelper.h>



class rtspTalkServer : public rt::TalkServer
{
using super = rt::TalkServer;
public:
    rtDefSingleton(rtspTalkServer);
    rtspTalkServer();
    ~rtspTalkServer();
    void addMessage(MessagePtr mes) override;
    bool onStats(StatsMessage& mes) override;
    bool onTalk(TalkMessage& mes) override;
    bool onStop(StopMessage& mes) override;
    bool ready() override;
#ifdef rtDebug
    bool onDebug(DebugMessage& mes) override;
#endif

    void wait();
    static void sampleCallbackS(const rt::TalkSample *data, void *userdata);
    void sampleCallback(const rt::TalkSample *data);

private:
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;

    rt::CastList m_casts;
    std::vector<CComPtr<ISpObjectToken>> m_voice_tokens;
    CComPtr<ISpVoice> m_voice;

    std::future<void> m_task_talk;
};


rtspTalkServer::rtspTalkServer()
{
    HRESULT hr = m_voice.CoCreateInstance(CLSID_SpVoice);
    if (FAILED(hr)) {
        return;
    }

    CComPtr<IEnumSpObjectTokens> cpEnum;
    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);
    if (FAILED(hr)) {
        return;
    }

    ULONG n;
    hr = cpEnum->GetCount(&n);
    int id_seed = 0;
    while (SUCCEEDED(hr) && n--)
    {
        CComPtr<ISpObjectToken> cpVoiceToken;
        hr = cpEnum->Next(1, &cpVoiceToken, NULL);
        if (SUCCEEDED(hr)) {
            PWSTR str;
            SpGetDescription(cpVoiceToken, &str);
            auto desc = rt::ToMBS(str);
            m_voice_tokens.push_back(cpVoiceToken);
            m_casts.push_back({ id_seed++, desc });
        }
    }
}

rtspTalkServer::~rtspTalkServer()
{
    wait();
}

void rtspTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);
    processMessages();
}

bool rtspTalkServer::onStats(StatsMessage& mes)
{
    if (!m_voice)
        return true;

    mes.stats.params.setCast(0);

    USHORT volume;
    m_voice->GetVolume(&volume);
    mes.stats.params.setVolume((float)volume/ 100.0f);
    //mes.stats.params.setVolume();

    mes.stats.host = "Windows SAPI";
    mes.stats.protocol_version = rtProtocolVersion;
    mes.stats.plugin_version = rtPluginVersion;
    mes.stats.casts = m_casts;
    return true;
}

bool rtspTalkServer::onTalk(TalkMessage& mes)
{
    if (!m_voice)
        return true;

    wait();

    if (mes.params.flags.volume)
        m_voice->SetVolume((USHORT)(mes.params.volume * 100.0f));
    if (mes.params.flags.cast && mes.params.cast < (int)m_voice_tokens.size())
        m_voice->SetVoice(m_voice_tokens[mes.params.cast]);

    auto text = rt::ToWCS(mes.text);
    m_task_talk = std::async(std::launch::async, [this, text]() {
        m_voice->Speak(text.c_str(), 0, nullptr);
    });

    return true;
}

bool rtspTalkServer::onStop(StopMessage& mes)
{
    if (!m_voice)
        return true;
    m_voice->Pause();
    return true;
}

bool rtspTalkServer::ready()
{
    if (!m_voice)
        return false;
    return true;
}

#ifdef rtDebug
bool rtspTalkServer::onDebug(DebugMessage& mes)
{
    return true;
}
void rtspTalkServer::wait()
{
    if (m_task_talk.valid())
        m_task_talk.wait();
}
#endif



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (FAILED(::CoInitialize(NULL)))
            return FALSE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}

rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return nullptr;
}

rtExport rt::TalkServer* rtspStartServer(int port)
{
    auto& inst = rtspTalkServer::getInstance();
    if (!inst.isRunning()) {
        rt::TalkServerSettings settings;
        settings.port = (uint16_t)port;
        inst.setSettings(settings);
        inst.start();
    }
    return &inst;
}
