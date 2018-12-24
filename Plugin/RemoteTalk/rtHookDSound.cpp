#include "pch.h"
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtHookDSound.h"
#ifdef _WIN32


namespace rt {

static std::vector<DSoundHandlerBase*>& GetDSoundHandlers()
{
    static std::vector<DSoundHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetDSoundHandlers().back()->Name(__VA_ARGS__);

static void SetHook_IDirectSound8(IDirectSound8 *dst);
static void SetHook_IDirectSoundBuffer(IDirectSoundBuffer *dst);

static HRESULT(WINAPI *DirectSoundCreate_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter);
static HRESULT WINAPI DirectSoundCreate_hook(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
{
    HRESULT ret;
    Call(onDirectSoundCreate, pcGuidDevice, ppDS, pUnkOuter, ret);
    return ret;
}

static HRESULT(WINAPI *DirectSoundCreate8_orig)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
static HRESULT WINAPI DirectSoundCreate8_hook(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter)
{
    HRESULT ret;
    Call(onDirectSoundCreate8, pcGuidDevice, ppDS8, pUnkOuter, ret);
    return ret;
}

static HRESULT(WINAPI *DirectSoundFullDuplexCreate_orig)(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd, DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter);
static HRESULT WINAPI DirectSoundFullDuplexCreate_hook(LPCGUID pcGuidCaptureDevice, LPCGUID pcGuidRenderDevice, LPCDSCBUFFERDESC pcDSCBufferDesc, LPCDSBUFFERDESC pcDSBufferDesc, HWND hWnd, DWORD dwLevel, LPDIRECTSOUNDFULLDUPLEX* ppDSFD, LPDIRECTSOUNDCAPTUREBUFFER8 *ppDSCBuffer8, LPDIRECTSOUNDBUFFER8 *ppDSBuffer8, LPUNKNOWN pUnkOuter)
{
    auto ret = DirectSoundFullDuplexCreate_orig(pcGuidCaptureDevice, pcGuidRenderDevice, pcDSCBufferDesc, pcDSBufferDesc, hWnd, dwLevel, ppDSFD, ppDSCBuffer8, ppDSBuffer8, pUnkOuter);
    //Call(afterDirectSoundCreate8, pcGuidDevice, ppDS8, pUnkOuter, ret);
    return ret;
}


// IDirectSound8
static ULONG(WINAPI *IDirectSound8_AddRef_orig)(IDirectSound8 *_this);
static ULONG WINAPI IDirectSound8_AddRef_hook(IDirectSound8 *_this)
{
    ULONG ret;
    Call(onIDirectSound8_AddRef, _this, ret);
    return ret;
}

static ULONG(WINAPI *IDirectSound8_Release_orig)(IDirectSound8 *_this);
static ULONG WINAPI IDirectSound8_Release_hook(IDirectSound8 *_this)
{
    ULONG ret;
    Call(onIDirectSound8_Release, _this, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSound8_CreateSoundBuffer_orig)(IDirectSound8 *_this, LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter);
static HRESULT WINAPI IDirectSound8_CreateSoundBuffer_hook(IDirectSound8 *_this, LPCDSBUFFERDESC pcDSBufferDesc, LPDIRECTSOUNDBUFFER *ppDSBuffer, LPUNKNOWN pUnkOuter)
{
    HRESULT ret;
    Call(onIDirectSound8_CreateSoundBuffer, _this, pcDSBufferDesc, ppDSBuffer, pUnkOuter, ret);
    return ret;
}

static void SetHook_IDirectSound8(IDirectSound8 *dst)
{
    if (!dst || IDirectSound8_CreateSoundBuffer_orig != nullptr)
        return;

    void **&vtable = ((void***)dst)[0];

#define Hook(Name, N)\
    (void*&)IDirectSound8_##Name##_orig = vtable[N];\
    ForceWrite(vtable[N], (void*)&IDirectSound8_##Name##_hook);

    Hook(AddRef, 1);
    Hook(Release, 2);
    Hook(CreateSoundBuffer, 3);
#undef Hook
}


// IDirectSoundBuffer

static ULONG(WINAPI *IDirectSoundBuffer_AddRef_orig)(IDirectSoundBuffer *_this);
static ULONG WINAPI IDirectSoundBuffer_AddRef_hook(IDirectSoundBuffer *_this)
{
    ULONG ret = 0;
    Call(onIDirectSoundBuffer_AddRef, _this, ret);
    return ret;
}

static ULONG(WINAPI *IDirectSoundBuffer_Release_orig)(IDirectSoundBuffer *_this);
static ULONG WINAPI IDirectSoundBuffer_Release_hook(IDirectSoundBuffer *_this)
{
    ULONG ret = 0;
    Call(onIDirectSoundBuffer_Release, _this, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_Lock_orig)(IDirectSoundBuffer *_this, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
static HRESULT WINAPI IDirectSoundBuffer_Lock_hook(IDirectSoundBuffer *_this, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_Lock, _this, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_Play_orig)(IDirectSoundBuffer *_this, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
static HRESULT WINAPI IDirectSoundBuffer_Play_hook(IDirectSoundBuffer *_this, DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_Play, _this, dwReserved1, dwPriority, dwFlags, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_SetCurrentPosition_orig)(IDirectSoundBuffer *_this, DWORD dwNewPosition);
static HRESULT WINAPI IDirectSoundBuffer_SetCurrentPosition_hook(IDirectSoundBuffer *_this, DWORD dwNewPosition)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_SetCurrentPosition, _this, dwNewPosition, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_SetFormat_orig)(IDirectSoundBuffer *_this, LPCWAVEFORMATEX pcfxFormat);
static HRESULT WINAPI IDirectSoundBuffer_SetFormat_hook(IDirectSoundBuffer *_this, LPCWAVEFORMATEX pcfxFormat)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_SetFormat, _this, pcfxFormat, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_SetVolume_orig)(IDirectSoundBuffer *_this, LONG lVolume);
static HRESULT WINAPI IDirectSoundBuffer_SetVolume_hook(IDirectSoundBuffer *_this, LONG lVolume)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_SetVolume, _this, lVolume, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_SetPan_orig)(IDirectSoundBuffer *_this, LONG lPan);
static HRESULT WINAPI IDirectSoundBuffer_SetPan_hook(IDirectSoundBuffer *_this, LONG lPan)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_SetPan, _this, lPan, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_SetFrequency_orig)(IDirectSoundBuffer *_this, DWORD dwFrequency);
static HRESULT WINAPI IDirectSoundBuffer_SetFrequency_hook(IDirectSoundBuffer *_this, DWORD dwFrequency)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_SetFrequency, _this, dwFrequency, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_Stop_orig)(IDirectSoundBuffer *_this);
static HRESULT WINAPI IDirectSoundBuffer_Stop_hook(IDirectSoundBuffer *_this)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_Stop, _this, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_Unlock_orig)(IDirectSoundBuffer *_this, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
static HRESULT WINAPI IDirectSoundBuffer_Unlock_hook(IDirectSoundBuffer *_this, LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_Unlock, _this, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2, ret);
    return ret;
}

static HRESULT(WINAPI *IDirectSoundBuffer_Restore_orig)(IDirectSoundBuffer *_this);
static HRESULT WINAPI IDirectSoundBuffer_Restore_hook(IDirectSoundBuffer *_this)
{
    HRESULT ret = 0;
    Call(onIDirectSoundBuffer_Restore, _this, ret);
    return ret;
}

static void SetHook_IDirectSoundBuffer(IDirectSoundBuffer *dst)
{
    if (!dst || IDirectSoundBuffer_Lock_orig != nullptr)
        return;

    void **&vtable = ((void***)dst)[0];

#define Hook(Name, N)\
    (void*&)IDirectSoundBuffer_##Name##_orig = vtable[N];\
    ForceWrite(vtable[N], (void*)&IDirectSoundBuffer_##Name##_hook);

    Hook(AddRef, 1);
    Hook(Release, 2);
    Hook(Lock, 11);
    Hook(Play, 12);
    Hook(SetCurrentPosition, 13);
    Hook(SetFormat, 14);
    Hook(SetVolume, 15);
    Hook(SetPan, 16);
    Hook(SetFrequency, 17);
    Hook(Stop, 18);
    Hook(Unlock, 19);
    Hook(Restore, 20);
#undef Hook
}


class DSoundHandlerRoot : public DSoundHandlerBase
{
public:
    rtDefSingleton(DSoundHandlerRoot);
    DSoundHandlerRoot() { GetDSoundHandlers().push_back(this); }

    void onDirectSoundCreate(LPCGUID& pcGuidDevice, LPDIRECTSOUND *&ppDS, LPUNKNOWN& pUnkOuter, HRESULT& ret) override
    {
        ret = DirectSoundCreate_orig(pcGuidDevice, ppDS, pUnkOuter);
    }
    void onDirectSoundCreate8(LPCGUID& pcGuidDevice, LPDIRECTSOUND8 *&ppDS8, LPUNKNOWN& pUnkOuter, HRESULT& ret) override
    {
        ret = DirectSoundCreate8_orig(pcGuidDevice, ppDS8, pUnkOuter);
        SetHook_IDirectSound8(*ppDS8);
    }

    void onIDirectSound8_AddRef(IDirectSound8 *&_this, ULONG& ret) override
    {
        ret = IDirectSound8_AddRef_orig(_this);
    }
    void onIDirectSound8_Release(IDirectSound8 *&_this, ULONG& ret) override
    {
        ret = IDirectSound8_Release_orig(_this);
    }
    void onIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret) override
    {
        ret = IDirectSound8_CreateSoundBuffer_orig(_this, pcDSBufferDesc, ppDSBuffer, pUnkOuter);
        SetHook_IDirectSoundBuffer(*ppDSBuffer);
    }

    void onIDirectSoundBuffer_AddRef(IDirectSoundBuffer *&_this, ULONG& ret) override
    {
        ret = IDirectSoundBuffer_AddRef_orig(_this);
    }
    void onIDirectSoundBuffer_Release(IDirectSoundBuffer *&_this, ULONG& ret) override
    {
        ret = IDirectSoundBuffer_Release_orig(_this);
    }
    void onIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwOffset, DWORD& dwBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_Lock_orig(_this, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
    }
    void onIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_Play_orig(_this, dwReserved1, dwPriority, dwFlags);
    }
    void onIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_SetCurrentPosition_orig(_this, dwNewPosition);
    }
    void onIDirectSoundBuffer_SetFormat(IDirectSoundBuffer *&_this, LPCWAVEFORMATEX& pcfxFormat, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_SetFormat_orig(_this, pcfxFormat);
    }
    void onIDirectSoundBuffer_SetVolume(IDirectSoundBuffer *&_this, LONG& lVolume, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_SetVolume_orig(_this, lVolume);
    }
    void onIDirectSoundBuffer_SetPan(IDirectSoundBuffer *&_this, LONG& lPan, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_SetPan_orig(_this, lPan);
    }
    void onIDirectSoundBuffer_SetFrequency(IDirectSoundBuffer *&_this, DWORD& dwFrequency, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_SetFrequency_orig(_this, dwFrequency);
    }
    void onIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_Stop_orig(_this);
    }
    void onIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_Unlock_orig(_this, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2);
    }
    void onIDirectSoundBuffer_Restore(IDirectSoundBuffer *&_this, HRESULT& ret) override
    {
        ret = IDirectSoundBuffer_Restore_orig(_this);
    }
};



static const GUID CLSID_DirectSound_ = { 0x47d4d946, 0x62e8, 0x11cf, 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 };
static const GUID CLSID_DirectSound8_ = { 0x3901cc3f, 0x84b5, 0x4fa4, 0xba, 0x35, 0xaa, 0x81, 0x72, 0xb8, 0xa0, 0x9b };
static const GUID IID_IDirectSound8_ = { 0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66 };


static bool InstallVTableHooks(HMODULE mod)
{
    HRESULT (WINAPI *_DirectSoundCreate8)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
    (void*&)_DirectSoundCreate8 = ::GetProcAddress(mod, "DirectSoundCreate8");
    if (!_DirectSoundCreate8)
        return false;

    IDirectSound8 *ds8 = nullptr;
    auto hr = _DirectSoundCreate8(nullptr, &ds8, nullptr);
    if (SUCCEEDED(hr)) {
        WAVEFORMATEX wh{};
        wh.cbSize = sizeof(WAVEFORMATEX);
        wh.wFormatTag = 1;
        wh.nChannels = 1;
        wh.nSamplesPerSec = 44100;
        wh.nAvgBytesPerSec = 88200;
        wh.nBlockAlign = 2;
        wh.wBitsPerSample = 16;

        DSBUFFERDESC desc{};
        desc.dwSize = sizeof(DSBUFFERDESC);
        desc.dwFlags = 0x0001c008;
        desc.dwBufferBytes = 352800;
        desc.lpwfxFormat = &wh;

        IDirectSoundBuffer *sbuf = nullptr;
        hr = ds8->CreateSoundBuffer(&desc, &sbuf, nullptr);
        if (SUCCEEDED(hr)) {
            SetHook_IDirectSound8(ds8);
            SetHook_IDirectSoundBuffer(sbuf);
            sbuf->Release();
            return true;
        }
        ds8->Release();
    }
    return false;
}


#define EachFunctions(Body)\
    Body(DirectSoundCreate);\
    Body(DirectSoundCreate8);\
    Body(DirectSoundFullDuplexCreate);

static const char DSound_DLL[] = "dsound.dll";

class LoadLibraryHandler_DSound : public LoadLibraryHandlerBase
{
using super = LoadLibraryHandlerBase;
public:
    rtDefSingleton(LoadLibraryHandler_DSound);

    void onLoadLibrary(HMODULE& mod) override
    {
        super::onLoadLibrary(mod);
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
};

class CoCreateHandler_DSound : public CoCreateHandlerBase
{
using super = CoCreateHandlerBase;
public:
    rtDefSingleton(CoCreateHandler_DSound);

    void onCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, REFIID riid, LPVOID *&ppv, HRESULT& ret) override
    {
        super::onCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv, ret);

        if (rclsid == CLSID_DirectSound_) {
            // nothing to do for now
        }
        else if (rclsid == CLSID_DirectSound8_) {
            SetHook_IDirectSound8(*(LPDIRECTSOUND8*)ppv);
        }
    }
};

bool InstallDSoundHook(HookType ht, bool load_dll)
{
    auto mod = ::GetModuleHandleA(DSound_DLL);
    if (!mod && load_dll)
        mod = ::LoadLibraryA(DSound_DLL);
    if (!mod)
        return false;

    void *tmp;
    if (ht == HookType::ATOverride) {
        auto jumptable = AllocExecutable(1024, mod);
#define Override(Name) tmp=OverrideEAT(mod, #Name, Name##_hook, jumptable); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override

        InstallLoadLibraryHook(HookType::ATOverride);
        AddLoadLibraryHandler(&LoadLibraryHandler_DSound::getInstance());

        InstallCoCreateHook(HookType::ATOverride);
        AddCoCreateHandler(&CoCreateHandler_DSound::getInstance());

        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_DSound::hook(mod); });
    }
    if (ht == HookType::Hotpatch) {
        if (!InstallVTableHooks(mod))
            return false;

#define Override(Name) tmp=Hotpatch(::GetProcAddress(mod, #Name), Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override
    }
    return true;
}

void AddDSoundHandler(DSoundHandlerBase *handler)
{
    DSoundHandlerRoot::getInstance();
    auto& handlers = GetDSoundHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

} // namespace rt
#endif // _WIN32
