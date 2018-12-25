#pragma once
#include "RemoteTalk/RemoteTalk.h"

namespace rtvrex {

class TalkInterface : public rt::TalkInterface
{
public:
    rtDefSingleton(TalkInterface);

    void release() override;
    const char* getClientName() const override;
    int getPluginVersion() const override;
    int getProtocolVersion() const override;

    bool getParams(rt::TalkParams& params) const override;
    int getNumCasts() const override;
    const rt::CastInfo* getCastInfo(int i) const override;

    bool setParams(const rt::TalkParams& params) override;
    bool setText(const char *text) override;

    bool isReady() const override;
    bool isPlaying() const override;
    bool play() override;
    bool stop() override;

public:
    // impl
    void setAudioCallback(const std::function<void(const rt::AudioData&)>& callback);

    bool isMainWindowVisible();
    bool prepareUI();
    void setupControls();
    void onSoundPlay();
    void onSoundStop();
    void onUpdateSample(const rt::AudioData& data);

private:
    std::wstring m_whost;
    std::string m_host;

    HWND m_ctrl_play = nullptr;
    HWND m_ctrl_stop = nullptr;
    HWND m_ctrl_text = nullptr;

    HWND m_ctl_tab = nullptr;
    HWND m_ctrl_volume = nullptr;
    HWND m_ctrl_speed = nullptr;
    HWND m_ctrl_pitch = nullptr;
    HWND m_ctrl_intonation = nullptr;

    rt::CastInfo m_cast;
    std::atomic_bool m_is_playing{ false };

    std::function<void(const rt::AudioData&)> m_callback;
};

} // namespace rtvrex
