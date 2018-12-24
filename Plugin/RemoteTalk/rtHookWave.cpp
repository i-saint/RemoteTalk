#include "pch.h"
#ifdef _WIN32
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtHookWave.h"

namespace rt {

static std::vector<WaveOutHandlerBase*>& GetWaveOutHandlers()
{
    static std::vector<WaveOutHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetWaveOutHandlers().back()->Name(__VA_ARGS__);

static MMRESULT(WINAPI *waveOutOpen_orig)(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen);
static MMRESULT WINAPI waveOutOpen_hook(LPHWAVEOUT phwo, UINT uDeviceID, LPCWAVEFORMATEX pwfx, DWORD_PTR dwCallback, DWORD_PTR dwInstance, DWORD fdwOpen)
{
    MMRESULT ret = 0;
    Call(onWaveOutOpen, phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutClose_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutClose_hook(HWAVEOUT hwo)
{
    MMRESULT ret = 0;
    Call(onWaveOutClose, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutPrepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutPrepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    MMRESULT ret = 0;
    Call(onWaveOutPrepareHeader, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutUnprepareHeader_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutUnprepareHeader_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    MMRESULT ret = 0;
    Call(onWaveOutUnprepareHeader, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutWrite_orig)(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh);
static MMRESULT WINAPI waveOutWrite_hook(HWAVEOUT hwo, LPWAVEHDR pwh, UINT cbwh)
{
    MMRESULT ret = 0;
    Call(onWaveOutWrite, hwo, pwh, cbwh, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutPause_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutPause_hook(HWAVEOUT hwo)
{
    MMRESULT ret = 0;
    Call(onWaveOutPause, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutRestart_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutRestart_hook(HWAVEOUT hwo)
{
    MMRESULT ret = 0;
    Call(onWaveOutRestart, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutReset_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutReset_hook(HWAVEOUT hwo)
{
    MMRESULT ret = 0;
    Call(onWaveOutReset, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutBreakLoop_orig)(HWAVEOUT hwo);
static MMRESULT WINAPI waveOutBreakLoop_hook(HWAVEOUT hwo)
{
    MMRESULT ret = 0;
    Call(onWaveOutBreakLoop, hwo, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutSetPitch_orig)(HWAVEOUT hwo, DWORD dwPitch);
static MMRESULT WINAPI waveOutSetPitch_hook(HWAVEOUT hwo, DWORD dwPitch)
{
    MMRESULT ret = 0;
    Call(onWaveOutSetPitch, hwo, dwPitch, ret);
    return ret;
}

static MMRESULT(WINAPI *waveOutSetPlaybackRate_orig)(HWAVEOUT hwo, DWORD dwRate);
static MMRESULT WINAPI waveOutSetPlaybackRate_hook(HWAVEOUT hwo, DWORD dwRate)
{
    MMRESULT ret = 0;
    Call(onWaveOutSetPlaybackRate, hwo, dwRate, ret);
    return ret;
}
#undef Call

class WaveOutHandlerRoot : public WaveOutHandlerBase
{
public:
    rtDefSingleton(WaveOutHandlerRoot);
    WaveOutHandlerRoot() { GetWaveOutHandlers().push_back(this); }

    void onWaveOutOpen(LPHWAVEOUT& phwo, UINT& uDeviceID, LPCWAVEFORMATEX& pwfx, DWORD_PTR& dwCallback, DWORD_PTR& dwInstance, DWORD& fdwOpen, MMRESULT& ret) override
    {
        ret = waveOutOpen_orig(phwo, uDeviceID, pwfx, dwCallback, dwInstance, fdwOpen);
    }
    void onWaveOutClose(HWAVEOUT& hwo, MMRESULT& ret) override
    {
        ret = waveOutClose_orig(hwo);
    }
    void onWaveOutPrepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) override
    {
        ret = waveOutPrepareHeader_orig(hwo, pwh, cbwh);
    }
    void onWaveOutUnprepareHeader(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) override
    {
        ret = waveOutUnprepareHeader_orig(hwo, pwh, cbwh);
    }
    void onWaveOutWrite(HWAVEOUT& hwo, LPWAVEHDR& pwh, UINT& cbwh, MMRESULT& ret) override
    {
        ret = waveOutWrite_orig(hwo, pwh, cbwh);
    }
    void onWaveOutPause(HWAVEOUT& hwo, MMRESULT& ret) override
    {
        ret = waveOutPause_orig(hwo);
    }
    void onWaveOutRestart(HWAVEOUT& hwo, MMRESULT& ret) override
    {
        ret = waveOutRestart_orig(hwo);
    }
    void onWaveOutReset(HWAVEOUT& hwo, MMRESULT& ret) override
    {
        ret = waveOutReset_orig(hwo);
    }
    void onWaveOutBreakLoop(HWAVEOUT& hwo, MMRESULT& ret) override
    {
        ret = waveOutBreakLoop_orig(hwo);
    }
    void onWaveOutSetPitch(HWAVEOUT& hwo, DWORD& dwPitch, MMRESULT& ret) override
    {
        ret = waveOutSetPitch_orig(hwo, dwPitch);
    }
    void onWaveOutSetPlaybackRate(HWAVEOUT& hwo, DWORD& dwRate, MMRESULT& ret) override
    {
        ret = waveOutSetPlaybackRate_orig(hwo, dwRate);
    }
};


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

    void onLoadLibrary(HMODULE& mod) override
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

bool InstallWaveOutHook(HookType ht, bool load_dll)
{
    auto mod = ::GetModuleHandleA(WinMM_DLL);
    if (!mod && load_dll)
        mod = ::LoadLibraryA(WinMM_DLL);
    if (!mod)
        return false;

    void *tmp;
    if (ht == HookType::ATOverride) {
        auto jumptable = AllocExecutable(1024, mod);
#define Override(Name) tmp=OverrideEAT(mod, #Name, Name##_hook, jumptable); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override

        InstallLoadLibraryHook(HookType::ATOverride);
        AddLoadLibraryHandler(&LoadLibraryHandler_WaveOut::getInstance());

        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_WaveOut::hook(mod); });
    }
    else if (ht == HookType::Hotpatch) {
#define Override(Name) tmp=Hotpatch(::GetProcAddress(mod, #Name), Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override
    }
    return true;
}

bool OverrideWaveOutIAT(HMODULE mod)
{
    if (!mod)
        return false;

    void *tmp;
#define Override(Name) tmp=OverrideIAT(mod, WinMM_DLL, #Name, Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
    EachFunctions(Override);
#undef Override
    return true;
}

void AddWaveOutHandler(WaveOutHandlerBase *handler)
{
    WaveOutHandlerRoot::getInstance();
    auto& handlers = GetWaveOutHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

} // namespace rt
#endif // _WIN32
