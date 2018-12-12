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

    bool talk();
    bool stop();
    bool wait();
    bool isPlaying();

private:
    static rtcvInterfaceManaged s_instance;

    ref class CastInfo
    {
    public:
        int id;
        String^ name;
        List<String^>^ param_names;

        CastInfo(int i, String^ n) : id(i), name(n), param_names(gcnew List<String^>()) {}
    };
    List<CastInfo^>^ m_casts;
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
    bool wait() override;

    void onPlay() override;
    void onStop() override;
    void onUpdateBuffer(rt::AudioData& ad) override;
#ifdef rtDebug
    bool onDebug() override;
#endif

private:
    mutable rt::CastList m_casts;
    rt::TalkParams m_params;

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
        for (int i = 0; i < list->Length; ++i) {
            auto info = gcnew CastInfo(i, list[i]);
            info->param_names->Add(L"‘å‚«‚³");
            info->param_names->Add(L"‘¬‚³");
            info->param_names->Add(L"‚‚³");
            info->param_names->Add(L"ºŽ¿");
            info->param_names->Add(L"—}—g");

            auto talker = gcnew Talker(list[i]);
            int n = talker->Components->Count;
            for (int i = 0; i < n; ++i)
                info->param_names->Add(talker->Components->At(i)->Name);

            m_casts->Add(info);
        }
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
        for each(auto ti in m_casts) {
            rt::CastInfoImpl ci;
            ci.id = ti->id;
            ci.name = ToStdString(ti->name);
            for each(auto pname in ti->param_names)
                ci.param_names.push_back(ToStdString(pname));
            ret.push_back(std::move(ci));
        }
    }
    return ret;
}

static inline float to_f(uint32_t v) { return (float)v / 100.0f; }
static inline uint32_t to_u(float v) { return (uint32_t)(v * 100.0f); }

bool rtcvInterfaceManaged::getParams(rt::TalkParams& params)
{
    updateCast();
    params.cast = m_cast;
    if (m_talker) {
        params.num_params = m_casts[m_cast]->param_names->Count;
        int pi = 0;
        params.params[pi++] = to_f(m_talker->Volume);
        params.params[pi++] = to_f(m_talker->Speed);
        params.params[pi++] = to_f(m_talker->Tone);
        params.params[pi++] = to_f(m_talker->ToneScale);
        params.params[pi++] = to_f(m_talker->Alpha);

        int n = m_talker->Components->Count;
        for (int i = 0; i < n; ++i) {
            params.params[pi++] = to_f(m_talker->Components->At(i)->Value);
            if (pi >= 12)
                break;
        }
    }
    return true;
}

bool rtcvInterfaceManaged::setParams(const rt::TalkParams& params)
{
    m_cast = params.cast;
    updateCast();
    if (!m_talker)
        return false;

    int pi = 0;
    m_talker->Volume = to_u(params.params[pi++]);
    m_talker->Speed = to_u(params.params[pi++]);
    m_talker->Tone = to_u(params.params[pi++]);
    m_talker->ToneScale = to_u(params.params[pi++]);
    m_talker->Alpha = to_u(params.params[pi++]);

    int n = m_talker->Components->Count;
    for (int i = 0; i < n; ++i) {
        m_talker->Components->At(i)->Value = to_u(params.params[pi++]);
        if (pi >= 12)
            break;
    }
    return true;
}

bool rtcvInterfaceManaged::setText(const char *text)
{
    m_text = gcnew String(text);
    return true;
}


bool rtcvInterfaceManaged::talk()
{
    stop();
    updateCast();

    if (!m_talker || !m_text || m_text->Length == 0)
        return false;

    m_state = m_talker->Speak(m_text);
    return m_state != nullptr;
}

bool rtcvInterfaceManaged::stop()
{
    if (m_state && !m_state->IsCompleted) {
        m_talker->Stop();
    }
    return true;
}

bool rtcvInterfaceManaged::wait()
{
    if (m_state) {
        m_state->Wait();
        return true;
    }
    return false;
}

bool rtcvInterfaceManaged::isPlaying()
{
    return m_state && !m_state->IsCompleted;
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
const char* rtcvTalkInterface::getClientName() const { return "CeVIO Creative Studio"; }
int rtcvTalkInterface::getPluginVersion() const { return rtPluginVersion; }
int rtcvTalkInterface::getProtocolVersion() const { return rtProtocolVersion; }

bool rtcvTalkInterface::getParams(rt::TalkParams& params) const
{
    return rtcvInterfaceManaged::getInstance()->getParams(params);
}

bool rtcvTalkInterface::setParams(const rt::TalkParams& params)
{
    m_params = params;
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
        *dst = m_casts[i].toCastInfo();
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
    return !rtcvInterfaceManaged::getInstance()->isPlaying();
}

bool rtcvTalkInterface::talk(rt::TalkSampleCallback cb, void *userdata)
{
    m_sample_cb = cb;
    m_sample_cb_userdata = userdata;
    if (rtcvInterfaceManaged::getInstance()->talk()) {
        return true;
    }
    else {
        return false;
    }
}

bool rtcvTalkInterface::stop()
{
    return rtcvInterfaceManaged::getInstance()->stop();
}

bool rtcvTalkInterface::wait()
{
    return rtcvInterfaceManaged::getInstance()->wait();
}


void rtcvTalkInterface::onPlay()
{
}

void rtcvTalkInterface::onStop()
{
    if (m_sample_cb) {
        rt::AudioData dummy;
        auto sd = rt::ToTalkSample(dummy);
        m_sample_cb(&sd, m_sample_cb_userdata);
    }
}

void rtcvTalkInterface::onUpdateBuffer(rt::AudioData& ad)
{
    if (m_sample_cb && rtcvInterfaceManaged::getInstance()->isPlaying()) {
        if (m_params.force_mono)
            ad.convertToMono();

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
