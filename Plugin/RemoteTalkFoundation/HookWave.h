#pragma once
#include <mmsystem.h>

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
    virtual void beforeWaveOutGetPosition(HWAVEOUT& hwo, LPMMTIME& pmmt, UINT& cbmmt) {}
    virtual void afterWaveOutGetPosition(HWAVEOUT& hwo, LPMMTIME& pmmt, UINT& cbmmt, MMRESULT& ret) {}
    virtual void beforeWaveOutGetPitch(HWAVEOUT& hwo, LPDWORD& pdwPitch) {}
    virtual void afterWaveOutGetPitch(HWAVEOUT& hwo, LPDWORD& pdwPitch, MMRESULT& ret) {}
    virtual void beforeWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch) {}
    virtual void afterWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch, MMRESULT& ret) {}
    virtual void beforeWaveOutGetPlaybackRate(HWAVEOUT& hwo, LPDWORD& pdwRate) {}
    virtual void afterWaveOutGetPlaybackRate(HWAVEOUT& hwo, LPDWORD& pdwRate, MMRESULT& ret) {}
    virtual void beforeWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate) {}
    virtual void afterWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate, MMRESULT& ret) {}
    virtual void beforeWaveOutGetID(HWAVEOUT& hwo, LPUINT& puDeviceID) {}
    virtual void afterWaveOutGetID(HWAVEOUT& hwo, LPUINT& puDeviceID, MMRESULT& ret) {}
};
#pragma warning(pop)

bool AddWaveOutHandler(WaveOutHandlerBase *handler);

