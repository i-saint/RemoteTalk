#pragma once
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"

class rtvrexTalkServer : public rt::TalkServer
{
    using super = rt::TalkServer;
public:
    rtDefSingleton(rtvrexTalkServer);
    void addMessage(MessagePtr mes) override;

    bool isReady() override;
    Status onStats(StatsMessage& mes) override;
    Status onTalk(TalkMessage& mes) override;
    Status onStop(StopMessage& mes) override;
#ifdef rtDebug
    Status onDebug(DebugMessage& mes) override;
#endif

    void onSoundPlay();
    void onSoundStop();
    void onUpdateSample(const rt::AudioData& data);

private:
    std::mutex m_data_mutex;
    std::vector<rt::AudioDataPtr> m_data_queue;


private:
    void setupControls();
    bool doPlay();
    bool doStop();
    bool doSetText(const std::wstring& text);

    std::wstring m_host;
    HWND m_ctrl_play = nullptr;
    HWND m_ctrl_stop = nullptr;
    HWND m_ctrl_text = nullptr;
    HWND m_tab_voice = nullptr;

    HWND m_ctrl_volume = nullptr;
    HWND m_ctrl_speed = nullptr;
    HWND m_ctrl_pitch = nullptr;
    HWND m_ctrl_intonation = nullptr;

    rt::CastInfo m_cast;
    std::atomic_bool m_is_playing{false};
};


