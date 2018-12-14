#include "pch.h"
#ifdef _WIN32
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtHookWave.h"

namespace rt {

static std::vector<WaveOutHandlerBase*> g_waveouthandlers;
#define Call(Name, ...) for(auto *handler : g_waveouthandlers) { handler->Name(__VA_ARGS__); }

static MMRESULT(WINAPI *waveOutOpen_orig)(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
static MMRESULT WINAPI waveOutOpen_hook(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    Call(beforeWaveOutOpen, phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    auto ret = waveOutOpen_orig(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    Call(afterWaveOutOpen, phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutClose_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutClose_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutClose, hwo);
    auto ret = waveOutClose_orig(hwo);
    Call(afterWaveOutClose, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutPrepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutPrepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutPrepareHeader, hwo, pwh, cbwh);
    auto ret = waveOutPrepareHeader_orig(hwo, pwh, cbwh);
    Call(afterWaveOutPrepareHeader, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutUnprepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutUnprepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutUnprepareHeader, hwo, pwh, cbwh);
    auto ret = waveOutUnprepareHeader_orig(hwo, pwh, cbwh);
    Call(afterWaveOutUnprepareHeader, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutWrite_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    Call(beforeWaveOutWrite, hwo, pwh, cbwh);
    auto ret = waveOutWrite_orig(hwo, pwh, cbwh);
    Call(afterWaveOutWrite, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutPause_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutPause_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutPause, hwo);
    auto ret = waveOutPause_orig(hwo);
    Call(afterWaveOutPause, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutRestart_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutRestart_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutRestart, hwo);
    auto ret = waveOutRestart_orig(hwo);
    Call(afterWaveOutRestart, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutReset_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutReset_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutReset, hwo);
    auto ret = waveOutReset_orig(hwo);
    Call(afterWaveOutReset, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutBreakLoop_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutBreakLoop_hook(HWAVEOUT hwo)
{
    Call(beforeWaveOutBreakLoop, hwo);
    auto ret = waveOutBreakLoop_orig(hwo);
    Call(afterWaveOutBreakLoop, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutSetPitch_orig)(HWAVEOUT hwo, DWORD dwPitch);
static MMRESULT WINAPI waveOutSetPitch_hook(HWAVEOUT hwo, DWORD dwPitch)
{
    Call(beforeWaveOutSetPitch, hwo, dwPitch);
    auto ret = waveOutSetPitch_orig(hwo, dwPitch);
    Call(afterWaveOutSetPitch, hwo, dwPitch, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutSetPlaybackRate_orig)(HWAVEOUT hwo, DWORD dwRate);
static MMRESULT WINAPI waveOutSetPlaybackRate_hook(HWAVEOUT hwo, DWORD dwRate)
{
    Call(beforeWaveOutSetPlaybackRate, hwo, dwRate);
    auto ret = waveOutSetPlaybackRate_orig(hwo, dwRate);
    Call(afterWaveOutSetPlaybackRate, hwo, dwRate, ret);
    return ret;
}

#define EachFunctions(Body)\
    Body(waveOutOpen);\
    Body(waveOutClose);\
    Body(waveOutPrepareHeader);\
    Body(waveOutUnprepareHeader);\
    Body(waveOutWrite);\
    Body(waveOutPause);\
    Body(waveOutRestart);\
    Body(waveOutReset);\
    Body(waveOutBreakLoop);\
    Body(waveOutSetPitch);\
    Body(waveOutSetPlaybackRate);


static const char WinMM_DLL[] = "winmm.dll";

class LoadLibraryHandler_WaveOut : public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandler_WaveOut);

    void afterLoadLibrary(HMODULE& mod) override
    {
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
        if (!IsValidModule(mod))
            return;
#define Override(Name) OverrideIAT(mod, WinMM_DLL, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
};

bool AddWaveOutHandler(WaveOutHandlerBase *handler, bool load_dll, HookType ht)
{
    // setup hooks
    auto mod = load_dll ? ::LoadLibraryA(WinMM_DLL) : ::GetModuleHandleA(WinMM_DLL);
    if (!mod)
        return false;

    if (!waveOutOpen_orig) {
        if (ht == HookType::ATOverride) {
            auto jumptable = AllocExecutableForward(1024, mod);
#define Override(Name) (void*&)Name##_orig = OverrideEAT(mod, #Name, Name##_hook, jumptable)
            EachFunctions(Override);
#undef Override

            AddLoadLibraryHandler(&LoadLibraryHandler_WaveOut::getInstance());
            EnumerateModules([](HMODULE mod) { LoadLibraryHandler_WaveOut::hook(mod); });
        }
        else if (ht == HookType::Hotpatch) {
#define Override(Name) (void*&)Name##_orig = Hotpatch(::GetProcAddress(mod, #Name), Name##_hook)
            EachFunctions(Override);
#undef Override
        }
    }

    g_waveouthandlers.push_back(handler);
    return true;
}

} // namespace rt
#endif // _WIN32
