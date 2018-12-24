#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvTalkInterfaceManaged.h"

namespace rtcv {

std::string ToStdString(String^ str)
{
    IntPtr ptr = Marshal::StringToHGlobalAnsi(str);
    return rt::ToUTF8((const char*)ptr.ToPointer());
}


TalkInterfaceManaged^ TalkInterfaceManaged::getInstance()
{
    return %s_instance;
}


TalkInterfaceManaged::TalkInterfaceManaged()
{
}

static inline float to_f(uint32_t v) { return (float)v / 50.0f; }
static inline uint32_t to_u(float v) { return (uint32_t)(v * 50.0f); }

void TalkInterfaceManaged::updateStats()
{
    if (!m_casts) {
        m_casts = gcnew List<CastInfo^>();
        auto list = TalkerAgent::AvailableCasts;
        for (int i = 0; i < list->Length; ++i) {
            auto info = gcnew CastInfo(i, list[i]);
            info->params->Add(gcnew ParamInfo{ L"‘å‚«‚³", 1.0f, 0.0f, 2.0f });
            info->params->Add(gcnew ParamInfo{ L"‘¬‚³", 1.0f, 0.0f, 2.0f });
            info->params->Add(gcnew ParamInfo{ L"‚‚³", 1.0f, 0.0f, 2.0f });
            info->params->Add(gcnew ParamInfo{ L"ºŽ¿", 1.0f, 0.0f, 2.0f });
            info->params->Add(gcnew ParamInfo{ L"—}—g", 1.0f, 0.0f, 2.0f });

            auto talker = gcnew Talker(list[i]);
            int n = talker->Components->Count;
            for (int i = 0; i < n; ++i) {
                auto c = talker->Components->At(i);
                info->params->Add(gcnew ParamInfo{ c->Name, to_f(c->Value), 0.0f, 2.0f });
            }

            m_casts->Add(info);
        }
    }
}

void TalkInterfaceManaged::updateCast()
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

rt::CastList TalkInterfaceManaged::getCastList()
{
    if (!m_casts)
        updateStats();

    rt::CastList ret;
    if (m_casts) {
        for each(auto ti in m_casts) {
            rt::CastInfo ci;
            ci.id = ti->id;
            ci.name = ToStdString(ti->name);
            for each(auto p in ti->params)
                ci.params.push_back({ ToStdString(p->name), p->value, p->range_min, p->range_max });
            ret.push_back(std::move(ci));
        }
    }
    return ret;
}

bool TalkInterfaceManaged::getParams(rt::TalkParams& params)
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

bool TalkInterfaceManaged::setParams(const rt::TalkParams& params)
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

void TalkInterfaceManaged::setText(const char *text)
{
    m_text = gcnew String(text);
}

void TalkInterfaceManaged::setTempFilePath(const char *path)
{
    m_tmp_path = gcnew String(path);
}

bool TalkInterfaceManaged::talk()
{
    stop();
    updateCast();

    if (!m_talker || !m_text || m_text->Length == 0)
        return false;

    if (m_tmp_path != nullptr && m_tmp_path->Length != 0) {
        return m_talker->OutputWaveToFile(m_text, m_tmp_path);
    }
    else {
        m_state = m_talker->Speak(m_text);
        return m_state != nullptr;
    }
}

bool TalkInterfaceManaged::stop()
{
    if (m_state && !m_state->IsCompleted) {
        m_talker->Stop();
    }
    return true;
}

bool TalkInterfaceManaged::wait()
{
    if (m_state) {
        m_state->Wait();
        return true;
    }
    return false;
}

bool TalkInterfaceManaged::isPlaying()
{
    return m_state && !m_state->IsCompleted;
}

} // namespace rtcv
