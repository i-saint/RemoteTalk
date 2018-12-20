#include "pch.h"
#include <Commctrl.h>
#include "rtvrexCommon.h"
#include "rtvrexHookHandler.h"
#include "rtvrexTalkServer.h"


static void RequestUpdate()
{
    ::PostMessageW((HWND)0xffff, WM_TIMER, 0, 0);
}

void rtvrexTalkServer::addMessage(MessagePtr mes)
{
    super::addMessage(mes);

    // force call GetMessageW()
    RequestUpdate();
}

bool rtvrexTalkServer::isReady()
{
    return false;
}

rtvrexTalkServer::Status rtvrexTalkServer::onStats(StatsMessage& mes)
{
    auto& stats = mes.stats;
    stats.host = rt::ToMBS(m_host);
    stats.plugin_version = rtPluginVersion;
    stats.protocol_version = rtProtocolVersion;

    setupControls();
    mes.stats.casts.push_back(m_cast);

    return Status::Succeeded;
}

rtvrexTalkServer::Status rtvrexTalkServer::onTalk(TalkMessage& mes)
{
    setupControls();

    rtvrDSoundHandler::getInstance().mute = mes.params.mute;
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }

    doSetParams(mes.params);
    if (!doSetText(mes.text))
        return Status::Failed;
    if (!doPlay())
        return Status::Failed;

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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });
    return Status::Succeeded;
}

rtvrexTalkServer::Status rtvrexTalkServer::onStop(StopMessage& mes)
{
    setupControls();
    doStop();
    return Status::Succeeded;
}

#ifdef rtDebug
rtvrexTalkServer::Status rtvrexTalkServer::onDebug(DebugMessage& mes)
{
    setupControls();
    return Status::Succeeded;
}
#endif

void rtvrexTalkServer::onSoundPlay()
{

}

void rtvrexTalkServer::onSoundStop()
{
    m_is_playing = false;
    auto terminator = std::make_shared<rt::AudioData>();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(terminator);
    }
}

void rtvrexTalkServer::onUpdateSample(const rt::AudioData& data)
{
    if (!m_is_playing)
        return;

    auto tmp = std::make_shared<rt::AudioData>(data);
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}


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

static bool SetTabIndex(HWND hwnd, int idx)
{
    if (!hwnd)
        return false;
    ::SendMessageW(hwnd, TCM_SETCURFOCUS, idx, 0);
    return true;
}

void rtvrexTalkServer::setupControls()
{
    int nth_edit = 0;
    int nth_tab = 0;
    rt::EnumerateAllWindows([this, &nth_edit, &nth_tab](HWND hwnd) {
        wchar_t text[256];
        auto n = ::GetWindowTextW(hwnd, text, 256);

        wchar_t wclass[256];
        ::RealGetWindowClassW(hwnd, wclass, 256);
        if (wcsstr(wclass, L"WindowsForms10.Window")) {
            if (m_host.empty())
                m_host = text;
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
            if (nth_tab++ == 1)
                SetTabIndex(hwnd, 2);
        }

        //wchar_t buf[512];
        //swprintf(buf, L"%p [%s] %s\n", hwnd, wclass, text);
        //::OutputDebugStringW(buf);
    });

    if (m_cast.name.empty()) {
        auto cast_name = m_host;
        cast_name = std::regex_replace(cast_name, std::wregex(L"^VOICEROIDÅ{ ", std::regex_constants::icase), L"");
        cast_name = std::regex_replace(cast_name, std::wregex(L" EX$", std::regex_constants::icase), L"");
        cast_name = std::regex_replace(cast_name, std::wregex(L"Talk$", std::regex_constants::icase), L"");

        m_cast.name = rt::ToMBS(cast_name);
        m_cast.params.push_back({ rt::ToUTF8("âπó "), 1.0f, 0.0f, 2.0f });
        m_cast.params.push_back({ rt::ToUTF8("òbë¨"), 1.0f, 0.5f, 4.0f });
        m_cast.params.push_back({ rt::ToUTF8("çÇÇ≥"), 1.0f, 0.5f, 2.0f });
        m_cast.params.push_back({ rt::ToUTF8("ó}óg"), 1.0f, 0.0f, 2.0f });
    }
}

bool rtvrexTalkServer::doPlay()
{
    if (m_ctrl_play) {
        m_is_playing = true;
        EmulateClick(m_ctrl_play);
        return true;
    }
    return false;
}

bool rtvrexTalkServer::doStop()
{
    if (m_ctrl_stop) {
        EmulateClick(m_ctrl_stop);
        return true;
    }
    return false;
}

bool rtvrexTalkServer::doSetParams(const rt::TalkParams& params)
{
    return
        SetValue(m_ctrl_volume, params[0]) &&
        SetValue(m_ctrl_speed, params[1]) &&
        SetValue(m_ctrl_pitch, params[2]) &&
        SetValue(m_ctrl_intonation, params[3]);
}

bool rtvrexTalkServer::doSetText(const std::string& text)
{
    if (m_ctrl_text) {
        SetValue(m_ctrl_text, text);
        return true;
    }
    return false;
}
