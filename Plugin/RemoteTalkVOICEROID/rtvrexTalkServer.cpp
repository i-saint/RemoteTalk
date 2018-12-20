#include "pch.h"
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
    stats.host = rtvrexHostName;
    stats.plugin_version = rtPluginVersion;
    stats.protocol_version = rtProtocolVersion;
    return Status::Succeeded;
}

rtvrexTalkServer::Status rtvrexTalkServer::onTalk(TalkMessage& mes)
{
    rtvrDSoundHandler::getInstance().mute = mes.params.mute;
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.clear();
    }

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
    return Status::Succeeded;
}

#ifdef rtDebug

std::wstring m_host;
HWND m_ctrl_play, m_ctrl_stop, m_ctrl_text, m_tab_voice;

static void PrintWindows()
{
    rt::EnumerateAllWindows([](HWND hwnd) {
        wchar_t text[256];
        auto n = ::GetWindowTextW(hwnd, text, 256);

        wchar_t wclass[256];
        ::RealGetWindowClassW(hwnd, wclass, 256);
        if (wcsstr(wclass, L"WindowsForms10.Window")) {
            if (m_host.empty())
                m_host = text;
            else if (wcsstr(text, L"âπê∫å¯â "))
                m_tab_voice = hwnd;
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


        wchar_t buf[512];
        swprintf(buf, L"%p [%s] %s\n", hwnd, wclass, text);
        ::OutputDebugStringW(buf);
    });

    static const char* text = "éÑÅAêÊîyÇÃÇ±Ç∆çDÇ´Ç≈Ç∑ÇÊÅH";
    ::SendMessageA(m_ctrl_text, WM_SETTEXT, 0, (LPARAM)text);
    ::SendMessageA(m_ctrl_play, BM_CLICK, 0, 0);
}

rtvrexTalkServer::Status rtvrexTalkServer::onDebug(DebugMessage& mes)
{
    PrintWindows();
    return Status::Succeeded;
}
#endif


void rtvrexTalkServer::onUpdateSample(const rt::AudioData& data)
{
    auto tmp = std::make_shared<rt::AudioData>(data);
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(tmp);
    }
}

void rtvrexTalkServer::onStop()
{
    auto terminator = std::make_shared<rt::AudioData>();
    {
        std::unique_lock<std::mutex> lock(m_data_mutex);
        m_data_queue.push_back(terminator);
    }
}
