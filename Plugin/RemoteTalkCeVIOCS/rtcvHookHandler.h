#pragma once

namespace rtcv {

class WindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    rtDefSingleton(WindowMessageHandler);
    void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
};

class WaveOutHandler : public rt::WaveOutHandlerBase
{
public:
    rtDefSingleton(WaveOutHandler);

    bool mute = false;
    std::function<void(rt::AudioData&)> onUpdate;

    void clearCallbacks();

protected:
    void afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) override;
    void beforeWaveOutClose(HWAVEOUT& hwo) override;
    void beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) override;
    void beforeWaveOutReset(HWAVEOUT& hwo) override;

private:
    struct Record
    {
        rt::AudioData data;
        bool is_opened = false;
        bool is_playing = false;
    };
    std::map<HWAVEOUT, Record> m_records;
};

} // namespace rtcv
