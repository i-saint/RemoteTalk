#include "pch.h"
#include "rtvrexTalkInterface.h"

namespace rtvrex {

static bool EmulateClick(HWND hwnd)
{
    if (!hwnd)
        return false;
    ::SendMessageW(hwnd, BM_CLICK, 0, 0);
    return true;
}

static bool SetValue(HWND hwnd, const std::string& value)
{
    if (!hwnd)
        return false;
    auto t = rt::ToWCS(rt::ToUTF8(value));
    ::SendMessageW(hwnd, WM_SETTEXT, 0, (LPARAM)t.c_str());
    ::SendMessageW(hwnd, WM_ACTIVATE, 1, 0);
    return true;
}

static bool SetValue(HWND hwnd, float value)
{
    if (!hwnd)
        return false;
    auto t = rt::to_string(value);
    ::SendMessageA(hwnd, WM_SETTEXT, 0, (LPARAM)t.c_str());
    ::SendMessageW(hwnd, WM_ACTIVATE, 1, 0);
    return true;
}

static bool GetValue(HWND hwnd, float& value)
{
    if (!hwnd)
        return false;
    char buf[256];
    ::SendMessageA(hwnd, WM_GETTEXT, 0, (LPARAM)buf);
    return sscanf(buf, "%f", &value) == 1;
}

static bool GetTabIndex(HWND hwnd, int& idx)
{
    if (!hwnd)
        return false;
    idx = ::SendMessageW(hwnd, TCM_GETCURFOCUS, idx, 0);
    return true;
}

static bool SetTabIndex(HWND hwnd, int idx)
{
    if (!hwnd)
        return false;
    ::SendMessageW(hwnd, TCM_SETCURFOCUS, idx, 0);
    return true;
}


void TalkInterface::setAudioCallback(const std::function<void(const rt::AudioData&)>& callback)
{
    m_callback = callback;
}

void TalkInterface::setupControls()
{
    int nth_edit = 0;
    int nth_tab = 0;
    rt::EnumerateAllWindows([this, &nth_edit, &nth_tab](HWND hwnd) {
        wchar_t text[256];
        auto n = ::GetWindowTextW(hwnd, text, 256);

        wchar_t wclass[256];
        ::RealGetWindowClassW(hwnd, wclass, 256);
        if (wcsstr(wclass, L"WindowsForms10.Window")) {
            if (m_whost.empty()) {
                m_whost = text;
                m_host = rt::ToMBS(m_whost);
            }
            else if (wcsstr(text, L"âπê∫å¯â "))
                EmulateClick(hwnd);
        }
        else if (wcsstr(wclass, L"WindowsForms10.BUTTON")) {
            if (wcsstr(text, L"çƒê∂") && !m_ctrl_play)
                m_ctrl_play = hwnd;
            else if (wcsstr(text, L"í‚é~") && !m_ctrl_stop)
                m_ctrl_stop = hwnd;
        }
        else if (wcsstr(wclass, L"WindowsForms10.RichEdit20W")) {
            if (!m_ctrl_text)
                m_ctrl_text = hwnd;
        }
        else if (wcsstr(wclass, L"WindowsForms10.EDIT") && !m_ctrl_volume) {
            switch (nth_edit++) {
            case 0: m_ctrl_intonation = hwnd; break;
            case 1: m_ctrl_pitch = hwnd; break;
            case 2: m_ctrl_speed = hwnd; break;
            case 3: m_ctrl_volume = hwnd; break;
            default: break;
            }
        }
        else if (wcsstr(wclass, L"WindowsForms10.SysTabControl32")) {
            if (nth_tab++ == 1) {
                m_ctl_tab = hwnd;
                SetTabIndex(hwnd, 2);
            }
        }

        //wchar_t buf[512];
        //swprintf(buf, L"%p [%s] %s\n", hwnd, wclass, text);
        //::OutputDebugStringW(buf);
    });

    if (m_cast.name.empty() && !m_whost.empty()) {
        auto cast_name = m_whost;
        cast_name = std::regex_replace(cast_name, std::wregex(L"^VOICEROIDÅ{ ", std::regex_constants::icase), L"");
        cast_name = std::regex_replace(cast_name, std::wregex(L" EX$", std::regex_constants::icase), L"");
        cast_name = std::regex_replace(cast_name, std::wregex(L"Talk$", std::regex_constants::icase), L"");

        m_cast.name = rt::ToMBS(cast_name);
        m_cast.params.clear();
        m_cast.params.push_back({ rt::ToUTF8("âπó "), 1.0f, 0.0f, 2.0f });
        m_cast.params.push_back({ rt::ToUTF8("òbë¨"), 1.0f, 0.5f, 4.0f });
        m_cast.params.push_back({ rt::ToUTF8("çÇÇ≥"), 1.0f, 0.5f, 2.0f });
        m_cast.params.push_back({ rt::ToUTF8("ó}óg"), 1.0f, 0.0f, 2.0f });
    }
}

void TalkInterface::release()
{
    // do nothing
}

const char* TalkInterface::getClientName() const
{
    return m_host.c_str();
}

int TalkInterface::getPluginVersion() const
{
    return rtPluginVersion;
}

int TalkInterface::getProtocolVersion() const
{
    return rtProtocolVersion;
}

bool TalkInterface::getParams(rt::TalkParams& params) const
{
    bool ret = false;
    float tmp = 0.0f;
    if (GetValue(m_ctrl_volume, tmp)) {
        params[0] = tmp;
        ret = true;
    }
    if (GetValue(m_ctrl_speed, tmp)) {
        params[1] = tmp;
        ret = true;
    }
    if (GetValue(m_ctrl_pitch, tmp)) {
        params[2] = tmp;
        ret = true;
    }
    if (GetValue(m_ctrl_intonation, tmp)) {
        params[3] = tmp;
        ret = true;
    }
    return ret;
}

int TalkInterface::getNumCasts() const
{
    return 1;
}

const rt::CastInfo* TalkInterface::getCastInfo(int i) const
{
    return i == 0 ? &m_cast : nullptr;
}

bool TalkInterface::setParams(const rt::TalkParams& params)
{
    bool ret = false;
    if (params.isSet(0))
        ret = SetValue(m_ctrl_volume, params[0]) || ret;
    if (params.isSet(1))
        ret = SetValue(m_ctrl_speed, params[1]) || ret;
    if (params.isSet(2))
        ret = SetValue(m_ctrl_pitch, params[2]) || ret;
    if (params.isSet(3))
        ret = SetValue(m_ctrl_intonation, params[3]) || ret;
    return ret;
}

bool TalkInterface::setText(const char *text)
{
    if (m_ctrl_text) {
        SetValue(m_ctrl_text, text);
        return true;
    }
    return false;
}

bool TalkInterface::isReady() const
{
    return m_ctrl_play && !m_is_playing;
}

bool TalkInterface::isPlaying() const
{
    return m_is_playing;
}

bool TalkInterface::play()
{
    if (EmulateClick(m_ctrl_play)) {
        m_is_playing = true;
        return true;
    }
    return false;
}

bool TalkInterface::stop()
{
    if (EmulateClick(m_ctrl_stop)) {
        return true;
    }
    return false;
}


void TalkInterface::onSoundPlay()
{
    // nothing to do for now.
    // m_is_playing is updated on play()
}

void TalkInterface::onSoundStop()
{
    if (!m_is_playing)
        return;

    m_is_playing = false;
    if (m_callback) {
        rt::AudioData terminator;
        m_callback(terminator);
    }
}

void TalkInterface::onUpdateSample(const rt::AudioData& data)
{
    if (!m_is_playing)
        return;

    if (m_callback)
        m_callback(data);
}

} // namespace rtvrex
