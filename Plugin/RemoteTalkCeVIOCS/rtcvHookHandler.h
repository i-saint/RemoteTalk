#pragma once

namespace rtcv {

class WaveOutHandler : public rt::WaveOutHandlerBase
{
using super = rt::WaveOutHandlerBase;
public:
    rtDefSingleton(WaveOutHandler);

    bool mute = false;
    std::function<void(rt::AudioData&)> onUpdate;

    void clearCallbacks();

protected:
    void onWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) override;
    void onWaveOutClose(HWAVEOUT& hwo, MMRESULT& ret) override;
    void onWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) override;
    void onWaveOutReset(HWAVEOUT& hwo, MMRESULT& ret) override;

private:
    struct Record
    {
        rt::AudioData data;
        bool is_opened = false;
        bool is_playing = false;
    };
    std::map<HWAVEOUT, Record> m_records;
};

class WindowMessageHandler : public rt::WindowMessageHandlerBase
{
    using super = rt::WindowMessageHandlerBase;
public:
    rtDefSingleton(WindowMessageHandler);
    void onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;

    const int interval = 33;
    int frame = 0;
    UINT_PTR timer_id = 0;
};

} // namespace rtcv
