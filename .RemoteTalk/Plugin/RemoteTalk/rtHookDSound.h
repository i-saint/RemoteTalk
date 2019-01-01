#pragma once
#ifdef _WIN32
#include <dsound.h>

namespace rt {

class DSoundHandlerBase
{
public:
    DSoundHandlerBase *prev = nullptr;

    virtual ~DSoundHandlerBase() {}
    virtual void onDirectSoundCreate(LPCGUID& pcGuidDevice, LPDIRECTSOUND *&ppDS, LPUNKNOWN& pUnkOuter, HRESULT& ret) { if (prev) prev->onDirectSoundCreate(pcGuidDevice, ppDS, pUnkOuter, ret); }
    virtual void onDirectSoundCreate8(LPCGUID& pcGuidDevice, LPDIRECTSOUND8 *&ppDS8, LPUNKNOWN& pUnkOuter, HRESULT& ret) { if (prev) prev->onDirectSoundCreate8(pcGuidDevice, ppDS8, pUnkOuter, ret); }

    virtual void onIDirectSound8_AddRef(IDirectSound8 *&_this, ULONG& ret) { if (prev) prev->onIDirectSound8_AddRef(_this, ret); }
    virtual void onIDirectSound8_Release(IDirectSound8 *&_this, ULONG& ret) { if (prev) prev->onIDirectSound8_Release(_this, ret); }
    virtual void onIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret) { if (prev) prev->onIDirectSound8_CreateSoundBuffer(_this, pcDSBufferDesc, ppDSBuffer, pUnkOuter, ret); }

    virtual void onIDirectSoundBuffer_AddRef(IDirectSoundBuffer *&_this, ULONG& ret) { if (prev) prev->onIDirectSoundBuffer_AddRef(_this, ret); }
    virtual void onIDirectSoundBuffer_Release(IDirectSoundBuffer *&_this, ULONG& ret) { if (prev) prev->onIDirectSoundBuffer_Release(_this, ret); }
    virtual void onIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwOffset, DWORD& dwBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_Lock(_this, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags, ret); }
    virtual void onIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_Play(_this, dwReserved1, dwPriority, dwFlags, ret); }
    virtual void onIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_SetCurrentPosition(_this, dwNewPosition, ret); }
    virtual void onIDirectSoundBuffer_SetFormat(IDirectSoundBuffer *&_this, LPCWAVEFORMATEX& pcfxFormat, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_SetFormat(_this, pcfxFormat, ret); }
    virtual void onIDirectSoundBuffer_SetVolume(IDirectSoundBuffer *&_this, LONG& lVolume, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_SetVolume(_this, lVolume, ret); }
    virtual void onIDirectSoundBuffer_SetPan(IDirectSoundBuffer *&_this, LONG& lPan, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_SetPan(_this, lPan, ret); }
    virtual void onIDirectSoundBuffer_SetFrequency(IDirectSoundBuffer *&_this, DWORD& dwFrequency, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_SetFrequency(_this, dwFrequency, ret); }
    virtual void onIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_Stop(_this, ret); }
    virtual void onIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_Unlock(_this, pvAudioPtr1, dwAudioBytes1, pvAudioPtr2, dwAudioBytes2, ret); }
    virtual void onIDirectSoundBuffer_Restore(IDirectSoundBuffer *&_this, HRESULT& ret) { if (prev) prev->onIDirectSoundBuffer_Restore(_this, ret); }
};


bool InstallDSoundHook(HookType ht, bool load_dll = true);
void AddDSoundHandler(DSoundHandlerBase *handler);

} // namespace rt
#endif // _WIN32
