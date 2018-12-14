#pragma once
#ifdef _WIN32
#include <dsound.h>

namespace rt {

#pragma warning(push)
#pragma warning(disable:4100)
class DSoundHandlerBase
{
public:
    virtual ~DSoundHandlerBase() {}
    virtual void afterDirectSoundCreate(LPCGUID& pcGuidDevice, LPDIRECTSOUND *&ppDS, LPUNKNOWN& pUnkOuter, HRESULT& ret) {}
    virtual void afterDirectSoundCreate8(LPCGUID& pcGuidDevice, LPDIRECTSOUND8 *&ppDS8, LPUNKNOWN& pUnkOuter, HRESULT& ret) {}
    virtual void afterCCIDirectSound(LPDIRECTSOUND *&ppDS, HRESULT& ret) {}
    virtual void afterCCIDirectSound8(LPDIRECTSOUND8 *&ppDS8, HRESULT& ret) {}

    virtual void afterIDirectSound8_AddRef(IDirectSound8 *&_this, ULONG& ret) {}
    virtual void afterIDirectSound8_Release(IDirectSound8 *&_this, ULONG& ret) {}
    virtual void afterIDirectSound8_CreateSoundBuffer(IDirectSound8 *&_this, LPCDSBUFFERDESC& pcDSBufferDesc, LPDIRECTSOUNDBUFFER *&ppDSBuffer, LPUNKNOWN& pUnkOuter, HRESULT& ret) {}

    virtual void afterIDirectSoundBuffer_AddRef(IDirectSoundBuffer *&_this, ULONG& ret) {}
    virtual void afterIDirectSoundBuffer_Release(IDirectSoundBuffer *&_this, ULONG& ret) {}
    virtual void afterIDirectSoundBuffer_Lock(IDirectSoundBuffer *&_this, DWORD& dwOffset, DWORD& dwBytes, LPVOID *&ppvAudioPtr1, LPDWORD& pdwAudioBytes1, LPVOID *&ppvAudioPtr2, LPDWORD& pdwAudioBytes2, DWORD& dwFlags, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_Play(IDirectSoundBuffer *&_this, DWORD& dwReserved1, DWORD& dwPriority, DWORD& dwFlags, HRESULT& ret) {}
    virtual void beforeIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition) {}
    virtual void afterIDirectSoundBuffer_SetCurrentPosition(IDirectSoundBuffer *&_this, DWORD& dwNewPosition, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_SetFormat(IDirectSoundBuffer *&_this, LPCWAVEFORMATEX& pcfxFormat, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_SetVolume(IDirectSoundBuffer *&_this, LONG& lVolume, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_SetPan(IDirectSoundBuffer *&_this, LONG& lPan, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_SetFrequency(IDirectSoundBuffer *&_this, DWORD& dwFrequency, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_Stop(IDirectSoundBuffer *&_this, HRESULT& ret) {}
    virtual void beforeIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2) {}
    virtual void afterIDirectSoundBuffer_Unlock(IDirectSoundBuffer *&_this, LPVOID& pvAudioPtr1, DWORD& dwAudioBytes1, LPVOID& pvAudioPtr2, DWORD& dwAudioBytes2, HRESULT& ret) {}
    virtual void afterIDirectSoundBuffer_Restore(IDirectSoundBuffer *&_this, HRESULT& ret) {}
};
#pragma warning(pop)

bool AddDSoundHandler(DSoundHandlerBase *handler, bool load_dll = true, HookType ht = HookType::ATOverride);

} // namespace rt
#endif // _WIN32
