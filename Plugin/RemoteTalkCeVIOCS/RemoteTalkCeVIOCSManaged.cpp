#include "pch.h"
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtNorm.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalk/rtTalkInterfaceImpl.h"
#include "RemoteTalk/rtSerialization.h"
#include "RemoteTalkCeVIOCSCommon.h"
#include <atomic>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace CeVIO::Talk::RemoteService;


ref class rtcvInterfaceManaged
{
public:
    static rtcvInterfaceManaged^ getInstance();

    rtcvInterfaceManaged();
    void updateStats();
    void updateCast();

    rt::CastList getCastList();
    bool getParams(rt::TalkParams& params);
    bool setParams(const rt::TalkParams& params);
    bool setText(const char *text);

    bool stop();
    bool talk();

private:
    static rtcvInterfaceManaged s_instance;

    ref class CastInfo
    {
    public:
        int id;
        String^ name;

        CastInfo(int i,  String^ n) : id(i), name(n) {}
    };
    List<CastInfo^>^ m_casts;
    float m_volume = 1.0f;
    float m_speed = 1.0f;
    float m_tone = 1.0f;
    int m_cast = 0;
    String^ m_text = "";

    Talker^ m_talker;
    SpeakingState^ m_state;
};

class rtcvTalkInterface : public rtcvITalkInterface
{
public:
    rtDefSingleton(rtcvTalkInterface);

    rtcvTalkInterface();
    ~rtcvTalkInterface() override;
    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    bool setParams(const rt::TalkParams& params) override;
    int getNumCasts() const override;
    bool getCastInfo(int i, rt::CastInfo *dst) const override;
    bool setText(const char *text) override;

    bool ready() const override;
    bool talk(rt::TalkSampleCallback cb, void *userdata) override;
    bool stop() override;

    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(const rt::AudioData& ad) override;
#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;

    std::atomic_bool m_is_playing{ false };
    rt::TalkSampleCallback m_sample_cb = nullptr;
    void *m_sample_cb_userdata = nullptr;
};



static std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return rt::ToUTF8((const char*)ptr.ToPointer());
}


rtcvInterfaceManaged^ rtcvInterfaceManaged::getInstance()
{
    return %s_instance;
}


rtcvInterfaceManaged::rtcvInterfaceManaged()
{
}

void rtcvInterfaceManaged::updateStats()
{
    if (!m_casts) {
        m_casts = gcnew List<CastInfo^>();
        auto list = TalkerAgent::AvailableCasts;
        for (int i = 0; i < list->Length; ++i)
            m_casts->Add(gcnew CastInfo(i, list[i]));
    }
}

void rtcvInterfaceManaged::updateCast()
{
    updateStats();
    if (!m_casts || m_casts->Count == 0)
        return;
    m_cast = rt::clamp(m_cast, 0, m_casts->Count);
    if (m_cast >= m_casts->Count)
        return;

    if (!m_talker || m_talker->Cast != m_casts[m_cast]->name)
        m_talker = gcnew Talker(m_casts[m_cast]->name);
}

rt::CastList rtcvInterfaceManaged::getCastList()
{
    if (!m_casts)
        updateStats();

    rt::CastList ret;
    if (m_casts) {
        for each(auto ti in m_casts)
            ret.push_back({ ti->id, ToStdString(ti->name) });
    }
    return ret;
}

bool rtcvInterfaceManaged::getParams(rt::TalkParams& params)
{
    updateCast();
    params.setCast(m_cast);
    if (m_talker) {
        params.setVolume((float)m_talker->Volume / 100.0f);
        params.setSpeed((float)m_talker->Speed / 100.0f);
        params.setPitch((float)m_talker->Tone / 100.0f);
        params.setIntonation((float)m_talker->ToneScale / 100.0f);
        params.setAlpha((float)m_talker->Alpha / 100.0f);

        int n = m_talker->Components->Count;
        for (int i = 0; i < n; ++i) {
            auto c = m_talker->Components->At(i);
            auto name = c->Name;
            auto val = (float)c->Value / 100.0f;
            if (name == L"Œ³‹C")
                params.setJoy(val);
            else if (name == L"•’Ê")
                params.setNormal(val);
            else if (name == L"“{‚è")
                params.setAnger(val);
            else if (name == L"ˆ£‚µ‚Ý")
                params.setSorrow(val);
        }
    }
    return true;
}

bool rtcvInterfaceManaged::setParams(const rt::TalkParams& params)
{
    if (params.flags.cast)
        m_cast = params.cast;
    return true;
}

bool rtcvInterfaceManaged::setText(const char *text)
{
    m_text = gcnew String(text);
    return true;
}


bool rtcvInterfaceManaged::stop()
{
    if (m_state && !m_state->IsCompleted) {
        m_talker->Stop();
    }
    return true;
}

bool rtcvInterfaceManaged::talk()
{
    stop();
    updateCast();

    if (!m_talker)
        return false;

    //m_talker->Volume = m_volume;
    //m_talker->Speed = m_speed;

    m_state = m_talker->Speak(m_text);
    return true;
}



rtcvTalkInterface::rtcvTalkInterface()
{
}

rtcvTalkInterface::~rtcvTalkInterface()
{
    auto mod = ::GetModuleHandleA("RemoteTalkVOICEROID2Hook.dll");
    if (mod) {
        void(*proc)();
        (void*&)proc = ::GetProcAddress(mod, "rtOnManagedModuleUnload");
        if (proc)
            proc();
    }
}

void rtcvTalkInterface::release() { /*do nothing*/ }
const char* rtcvTalkInterface::getClientName() const { return "CeVIO CS"; }
int rtcvTalkInterface::getPluginVersion() const { return rtPluginVersion; }
int rtcvTalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool rtcvTalkInterface::getParams(rt::TalkParams& params) const
{
    return rtcvInterfaceManaged::getInstance()->getParams(params);
}

bool rtcvTalkInterface::setParams(const rt::TalkParams& params)
{
    return rtcvInterfaceManaged::getInstance()->setParams(params);
}

int rtcvTalkInterface::getNumCasts() const
{
    if (m_casts.empty())
        m_casts = rtcvInterfaceManaged::getInstance()->getCastList();
    return (int)m_casts.size();
}

bool rtcvTalkInterface::getCastInfo(int i, rt::CastInfo *dst) const
{
    if (i < (int)m_casts.size()) {
        dst->id = m_casts[i].id;
        dst->name = m_casts[i].name.c_str();
        return true;
    }
    return false;
}

bool rtcvTalkInterface::setText(const char *text)
{
    return rtcvInterfaceManaged::getInstance()->setText(text);
}

bool rtcvTalkInterface::ready() const
{
    return !m_is_playing;
}

bool rtcvTalkInterface::talk(rt::TalkSampleCallback cb, void *userdata)
{
    if (m_is_playing)
        return false;

    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtcvInterfaceManaged::getInstance()->talk()) {
        m_is_playing = true;
        return true;
    }
    else {
        return false;
    }
}

bool rtcvTalkInterface::stop()
{
    if (!m_is_playing)
        return false;
    return rtcvInterfaceManaged::getInstance()->stop();
}


void rtcvTalkInterface::onPlay()
{
    m_is_playing = true;
}

void rtcvTalkInterface::onStop()
{
    if (m_sample_cb && m_is_playing) {
        rt::AudioData dummy;
        auto sd = rt::ToTalkSample(dummy);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
    m_is_playing = false;
}

void rtcvTalkInterface::onUpdateBuffer(const rt::AudioData& ad)
{
    if (m_sample_cb && m_is_playing) {
        auto sd = rt::ToTalkSample(ad);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
}


#ifdef rtDebug
bool rtcvTalkInterface::onDebug()
{
    return true;
}
#endif


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtcvTalkInterface::getInstance();
}
