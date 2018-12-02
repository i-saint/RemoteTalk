#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"

class DSoundHandler : public rt::DSoundHandlerBase
{
public:
    void afterCCIDirectSound8(LPDIRECTSOUND8 *&ppDS8, HRESULT& ret) override;
} static g_dsound_handler;

void DSoundHandler::afterCCIDirectSound8(LPDIRECTSOUND8 *& ppDS8, HRESULT & ret)
{
    printf("");
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        rt::AddDSoundHandler(&g_dsound_handler);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
