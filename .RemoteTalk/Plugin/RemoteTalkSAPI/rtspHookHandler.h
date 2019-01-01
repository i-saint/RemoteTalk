#pragma once
#include "rtspCommon.h"

namespace rtsp {

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
        WAVEFORMATEX wave_format;
        rt::AudioData data;
        bool is_opened = false;
        bool is_playing = false;
    };
    std::map<HWAVEOUT, Record> m_records;
};

} // namespace rtsp
