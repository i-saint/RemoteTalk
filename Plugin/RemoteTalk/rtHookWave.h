#pragma once
#ifdef _WIN32
#include <mmsystem.h>

namespace rt {

class WaveOutHandlerBase
{
public:
    WaveOutHandlerBase *prev;

    virtual ~WaveOutHandlerBase() {}
    virtual void onWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) { if (prev) prev->onWaveOutOpen(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen, ret); }
    virtual void onWaveOutClose(HWAVEOUT& hwo, MMRESULT& ret) { if (prev) prev->onWaveOutClose(hwo, ret); }
    virtual void onWaveOutPrepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) { if (prev) prev->onWaveOutPrepareHeader(hwo, pwh, cbwh, ret); }
    virtual void onWaveOutUnprepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) { if (prev) prev->onWaveOutUnprepareHeader(hwo, pwh, cbwh, ret); }
    virtual void onWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) { if (prev) prev->onWaveOutWrite(hwo, pwh, cbwh, ret); }
    virtual void onWaveOutPause(HWAVEOUT& hwo, MMRESULT& ret) { if (prev) prev->onWaveOutPause(hwo, ret); }
    virtual void onWaveOutRestart(HWAVEOUT& hwo, MMRESULT& ret) { if (prev) prev->onWaveOutRestart(hwo, ret); }
    virtual void onWaveOutReset(HWAVEOUT& hwo, MMRESULT& ret) { if (prev) prev->onWaveOutReset(hwo, ret); }
    virtual void onWaveOutBreakLoop(HWAVEOUT& hwo, MMRESULT& ret) { if (prev) prev->onWaveOutBreakLoop(hwo, ret); }
    virtual void onWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch, MMRESULT& ret) { if (prev) prev->onWaveOutSetPitch(hwo, dwPitch, ret); }
    virtual void onWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate, MMRESULT& ret) { if (prev) prev->onWaveOutSetPlaybackRate(hwo, dwRate, ret); }
};

bool InstallWaveOutHook(HookType ht, bool load_dll = true);
bool OverrideWaveOutIAT(HMODULE target);
void AddWaveOutHandler(WaveOutHandlerBase *handler);

} // namespace rt
#endif // _WIN32
