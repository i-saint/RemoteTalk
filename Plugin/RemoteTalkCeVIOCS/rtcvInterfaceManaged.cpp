#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvInterfaceManaged.h"


std::string ToStdString(String^ str)
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

static inline float to_f(uint32_t v) { return (float)v / 50.0f; }
static inline uint32_t to_u(float v) { return (uint32_t)(v * 50.0f); }

bool rtcvInterfaceManaged::getParams(rt::TalkParams& params)
{
    updateCast();
    params.cast = m_cast;
    if (m_talker) {
        int pi = 0;
        params[pi++] = to_f(m_talker->Volume);
        params[pi++] = to_f(m_talker->Speed);
        params[pi++] = to_f(m_talker->Tone);
        params[pi++] = to_f(m_talker->ToneScale);
        params[pi++] = to_f(m_talker->Alpha);

        int n = m_talker->Components->Count;
        for (int i = 0; i < n; ++i) {
            params[pi++] = to_f(m_talker->Components->At(i)->Value);
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
    if (params.isSet(pi)) { m_talker->Volume = to_u(params[pi]); } ++pi;
    if (params.isSet(pi)) { m_talker->Speed = to_u(params[pi]); } ++pi;
    if (params.isSet(pi)) { m_talker->Tone = to_u(params[pi]); } ++pi;
    if (params.isSet(pi)) { m_talker->ToneScale = to_u(params[pi]); } ++pi;
    if (params.isSet(pi)) { m_talker->Alpha = to_u(params[pi]); } ++pi;

    int n = m_talker->Components->Count;
    for (int i = 0; i < n; ++i) {
        if (params.isSet(pi)) { m_talker->Components->At(i)->Value = to_u(params[pi]); } ++pi;
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

