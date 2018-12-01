#include "pch.h"
#include "Hook.h"
#include "HookDSound.h"


#pragma warning(push)
#pragma warning(disable:4229)
static HRESULT (*WINAPI DirectSoundCreate_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
static HRESULT (*WINAPI DirectSoundCreate8_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
#pragma warning(pop)

static DSoundHandlerBase *g_dsoundhandler = nullptr;
#define Call(Name, ...) g_dsoundhandler->Name(__VA_ARGS__);

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

#define EachFunctions(Body)\
    Body(DirectSoundCreate);\
    Body(DirectSoundCreate8);

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
        if (!mod)
            return;
#define Override(Name) OverrideIAT(mod, DSound_DLL, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
} static g_loadlibraryhandler_dsound;

bool AddDSoundHandler(DSoundHandlerBase *handler, bool load_dll)
{
    g_dsoundhandler = handler;

    // setup hooks
    auto dsound = load_dll ? ::LoadLibraryA(DSound_DLL) : ::GetModuleHandleA(DSound_DLL);
    if (!dsound)
        return false;

    auto jumptable = AllocExecutableForward(1024, dsound);
#define Override(Name) (void*&)Name##_orig = OverrideEAT(dsound, #Name, Name##_hook, jumptable)
    EachFunctions(Override);
#undef Override

    EnumerateModules([](HMODULE mod) { LoadLibraryHandler_DSound::hook(mod); });
    AddLoadLibraryHandler(&g_loadlibraryhandler_dsound);
    return true;
}
