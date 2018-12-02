#include "pch.h"
#include "Hook.h"
#include "HookKernel.h"
#include "HookDSound.h"


static HRESULT(WINAPI *DirectSoundEnumerateA_orig)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
static HRESULT(WINAPI *DirectSoundCreate_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
static HRESULT(WINAPI *DirectSoundCreate8_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
static HRESULT(WINAPI *DirectSoundFullDuplexCreate_orig)(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd, DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter);

static std::vector<DSoundHandlerBase*> g_dsoundhandlers;
#define Call(Name, ...) for(auto *handler : g_dsoundhandlers) { handler->Name(__VA_ARGS__); }

static HRESULT WINAPI DirectSoundEnumerateA_hook(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext)
{
    auto ret = DirectSoundEnumerateA_orig(pDSEnumCallback, pContext);
    return ret;
}

static HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
    auto ret = DirectSoundCreate_orig(pcGuidDevice, ppDS, pUnkOuter);
    Call(afterDirectSoundCreate, pcGuidDevice, ppDS, pUnkOuter, ret);
    return ret;
}

static HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
    auto ret = DirectSoundCreate8_orig(pcGuidDevice, ppDS8, pUnkOuter);
    Call(afterDirectSoundCreate8, pcGuidDevice, ppDS8, pUnkOuter, ret);
    return ret;
}

static HRESULT WINAPI DirectSoundFullDuplexCreate_hook(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd, DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
    auto ret = DirectSoundFullDuplexCreate_orig(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);
    //Call(afterDirectSoundCreate8, pcGuidDevice, ppDS8, pUnkOuter, ret);
    return ret;
}


#define EachFunctions(Body)\
    Body(DirectSoundEnumerateA);\
    Body(DirectSoundCreate);\
    Body(DirectSoundCreate8);\
    Body(DirectSoundFullDuplexCreate);

static const char DSound_DLL[] = "dsound.dll";

class LoadLibraryHandler_DSound : public LoadLibraryHandlerBase
{
public:
    void afterLoadLibrary(HMODULE& mod) override
    {
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
        if (!IsValidModule(mod))
            return;
#define Override(Name) OverrideIAT(mod, DSound_DLL, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
} static g_loadlibraryhandler_dsound;

class CoCreateHandler_DSound : public CoCreateHandlerBase
{
public:
    void afterCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& /*pUnkOuter*/, DWORD& /*dwClsContext*/, REFIID /*riid*/, LPVOID *&ppv, HRESULT& ret) override
    {
        static const GUID CLSID_DirectSound_ = { 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 };
        static const GUID CLSID_DirectSound8_ = { 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b };

        if (rclsid == CLSID_DirectSound_) {
            Call(afterCCIDirectSound, (LPDIRECTSOUND*&)ppv, ret);
        }
        else if (rclsid == CLSID_DirectSound8_) {
            Call(afterCCIDirectSound8, (LPDIRECTSOUND8*&)ppv, ret);
        }
    }
} static g_cocreatehandler_dsound;

bool AddDSoundHandler(DSoundHandlerBase *handler, bool load_dll)
{
    g_dsoundhandlers.push_back(handler);

    // setup hooks
    auto dsound = load_dll ? ::LoadLibraryA(DSound_DLL) : ::GetModuleHandleA(DSound_DLL);
    if (!dsound)
        return false;

    if (!DirectSoundEnumerateA_orig) {
        auto jumptable = AllocExecutableForward(1024, dsound);
#define Override(Name) (void*&)Name##_orig = OverrideEAT(dsound, #Name, Name##_hook, jumptable)
        EachFunctions(Override);
#undef Override

        AddLoadLibraryHandler(&g_loadlibraryhandler_dsound);
        AddCoCreateHandler(&g_cocreatehandler_dsound);
        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_DSound::hook(mod); });
    }
    return true;
}
