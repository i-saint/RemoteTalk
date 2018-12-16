#pragma once
#ifdef _WIN32
#include <mmsystem.h>

namespace rt {

#pragma warning(push)
#pragma warning(disable:4100)
class WaveOutHandlerBase
{
public:
    virtual ~WaveOutHandlerBase() {}
    virtual void beforeWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen) {}
    virtual void afterWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) {}
    virtual void beforeWaveOutClose(HWAVEOUT& hwo) {}
    virtual void afterWaveOutClose(HWAVEOUT& hwo, MMRESULT& ret) {}
    virtual void beforeWaveOutPrepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) {}
    virtual void afterWaveOutPrepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) {}
    virtual void beforeWaveOutUnprepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) {}
    virtual void afterWaveOutUnprepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) {}
    virtual void beforeWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh) {}
    virtual void afterWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) {}
    virtual void beforeWaveOutPause(HWAVEOUT& hwo) {}
    virtual void afterWaveOutPause(HWAVEOUT& hwo, MMRESULT& ret) {}
    virtual void beforeWaveOutRestart(HWAVEOUT& hwo) {}
    virtual void afterWaveOutRestart(HWAVEOUT& hwo, MMRESULT& ret) {}
    virtual void beforeWaveOutReset(HWAVEOUT& hwo) {}
    virtual void afterWaveOutReset(HWAVEOUT& hwo, MMRESULT& ret) {}
    virtual void beforeWaveOutBreakLoop(HWAVEOUT& hwo) {}
    virtual void afterWaveOutBreakLoop(HWAVEOUT& hwo, MMRESULT& ret) {}
    virtual void beforeWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch) {}
    virtual void afterWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch, MMRESULT& ret) {}
    virtual void beforeWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate) {}
    virtual void afterWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate, MMRESULT& ret) {}
};
#pragma warning(pop)

bool AddWaveOutHandler(WaveOutHandlerBase *handler, bool load_dll = true, HookType ht = HookType::ATOverride);
bool OverrideWaveOutIAT(WaveOutHandlerBase *handler, HMODULE target);

} // namespace rt
#endif // _WIN32
