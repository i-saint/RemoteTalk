#pragma once
#define INITGUID
#include <dsound.h>

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
};
#pragma warning(pop)

bool AddDSoundHandler(DSoundHandlerBase *handler, bool load_dll = true);
