#include "pch.h"
#include "Hook.h"
#include "HookWave.h"


#pragma warning(push)
#pragma warning(disable:4229)
static MMRESULT (*WINAPI waveOutOpen_orig)(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
static MMRESULT (*WINAPI waveOutClose_orig)(HWAVEOUT hwo);
static MMRESULT (*WINAPI waveOutPrepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT (*WINAPI waveOutUnprepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT (*WINAPI waveOutWrite_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT (*WINAPI waveOutPause_orig)(HWAVEOUT hwo);
static MMRESULT (*WINAPI waveOutRestart_orig)(HWAVEOUT hwo);
static MMRESULT (*WINAPI waveOutReset_orig)(HWAVEOUT hwo);
static MMRESULT (*WINAPI waveOutBreakLoop_orig)(HWAVEOUT hwo);
static MMRESULT (*WINAPI waveOutGetPosition_orig)(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt);
static MMRESULT (*WINAPI waveOutGetPitch_orig)(HWAVEOUT hwo, LPDWORD pdwPitch);
static MMRESULT (*WINAPI waveOutSetPitch_orig)(HWAVEOUT hwo, DWORD dwPitch);
static MMRESULT (*WINAPI waveOutGetPlaybackRate_orig)(HWAVEOUT hwo, LPDWORD pdwRate);
static MMRESULT (*WINAPI waveOutSetPlaybackRate_orig)(HWAVEOUT hwo, DWORD dwRate);
static MMRESULT (*WINAPI waveOutGetID_orig)(HWAVEOUT hwo, LPUINT puDeviceID);
#pragma warning(pop)

static WaveOutHandlerBase *g_waveouthandler;
#define Call(Name, ...) if(g_waveouthandler) { g_waveouthandler->Name(__VA_ARGS__); }

static MMRESULT WINAPI waveOutOpen_hook(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Call(beforeWaveOutOpen, phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    auto ret = waveOutOpen_orig(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    Call(afterWaveOutOpen, phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    return ret;
}

static MMRESULT WINAPI waveOutClose_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutClose, hwo);
    auto ret = waveOutClose_orig(hwo);
    Call(afterWaveOutClose, hwo);
    return ret;
}

static MMRESULT WINAPI waveOutPrepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutPrepareHeader, hwo, pwh, cbwh);
    auto ret = waveOutPrepareHeader_orig(hwo, pwh, cbwh);
    Call(afterWaveOutPrepareHeader, hwo, pwh, cbwh);
    return ret;
}

static MMRESULT WINAPI waveOutUnprepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutUnprepareHeader, hwo, pwh, cbwh);
    auto ret = waveOutUnprepareHeader_orig(hwo, pwh, cbwh);
    Call(afterWaveOutUnprepareHeader, hwo, pwh, cbwh);
    return ret;
}

static MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutWrite, hwo, pwh, cbwh);
    auto ret = waveOutWrite_orig(hwo, pwh, cbwh);
    Call(afterWaveOutWrite, hwo, pwh, cbwh);
    return ret;
}

static MMRESULT WINAPI waveOutPause_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutPause, hwo);
    auto ret = waveOutPause_orig(hwo);
    Call(afterWaveOutPause, hwo);
    return ret;
}

static MMRESULT WINAPI waveOutRestart_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutRestart, hwo);
    auto ret = waveOutRestart_orig(hwo);
    Call(afterWaveOutRestart, hwo);
    return ret;
}

static MMRESULT WINAPI waveOutReset_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutReset, hwo);
    auto ret = waveOutReset_orig(hwo);
    Call(afterWaveOutReset, hwo);
    return ret;
}

static MMRESULT WINAPI waveOutBreakLoop_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutBreakLoop, hwo);
    auto ret = waveOutBreakLoop_orig(hwo);
    Call(afterWaveOutBreakLoop, hwo);
    return ret;
}

static MMRESULT WINAPI waveOutGetPosition_hook(HWAVEOUT hwo, LPMMTIME pmmt, UINT cbmmt)
{
    Call(beforeWaveOutGetPosition, hwo, pmmt, cbmmt);
    auto ret = waveOutGetPosition_orig(hwo, pmmt, cbmmt);
    Call(afterWaveOutGetPosition, hwo, pmmt, cbmmt);
    return ret;
}

static MMRESULT WINAPI waveOutGetPitch_hook(HWAVEOUT hwo, LPDWORD pdwPitch)
{
    Call(beforeWaveOutGetPitch, hwo, pdwPitch);
    auto ret = waveOutGetPitch_orig(hwo, pdwPitch);
    Call(afterWaveOutGetPitch, hwo, pdwPitch);
    return ret;
}

static MMRESULT WINAPI waveOutSetPitch_hook(HWAVEOUT hwo, DWORD dwPitch)
{
    Call(beforeWaveOutSetPitch, hwo, dwPitch);
    auto ret = waveOutSetPitch_orig(hwo, dwPitch);
    Call(afterWaveOutSetPitch, hwo, dwPitch);
    return ret;
}

static MMRESULT WINAPI waveOutGetPlaybackRate_hook(HWAVEOUT hwo, LPDWORD pdwRate)
{
    Call(beforeWaveOutGetPlaybackRate, hwo, pdwRate);
    auto ret = waveOutGetPlaybackRate_orig(hwo, pdwRate);
    Call(afterWaveOutGetPlaybackRate, hwo, pdwRate);
    return ret;
}

static MMRESULT WINAPI waveOutSetPlaybackRate_hook(HWAVEOUT hwo, DWORD dwRate)
{
    Call(beforeWaveOutSetPlaybackRate, hwo, dwRate);
    auto ret = waveOutSetPlaybackRate_orig(hwo, dwRate);
    Call(afterWaveOutSetPlaybackRate, hwo, dwRate);
    return ret;
}

static MMRESULT WINAPI waveOutGetID_hook(HWAVEOUT hwo, LPUINT puDeviceID)
{
    Call(beforeWaveOutGetID, hwo, puDeviceID);
    auto ret = waveOutGetID_orig(hwo, puDeviceID);
    Call(afterWaveOutGetID, hwo, puDeviceID);
    return ret;
}


bool HookWaveOutFunctions(WaveOutHandlerBase *handler)
{
    g_waveouthandler = handler;

    // setup hooks
    auto winmm = ::GetModuleHandleA("winmm.dll");
    if (!winmm)
        return false;

    auto jumptable = AllocateExecutableMemoryForward(1024, winmm);

#define Override(Name) (void*&)Name##_orig = OverrideDLLExport(winmm, #Name, Name##_hook, jumptable)
    Override(waveOutOpen);
    Override(waveOutClose);
    Override(waveOutPrepareHeader);
    Override(waveOutUnprepareHeader);
    Override(waveOutWrite);
    Override(waveOutPause);
    Override(waveOutRestart);
    Override(waveOutReset);
    Override(waveOutBreakLoop);
    Override(waveOutGetPosition);
    Override(waveOutGetPitch);
    Override(waveOutSetPitch);
    Override(waveOutGetPlaybackRate);
    Override(waveOutSetPlaybackRate);
    Override(waveOutGetID);
#undef Override
    return true;
}
