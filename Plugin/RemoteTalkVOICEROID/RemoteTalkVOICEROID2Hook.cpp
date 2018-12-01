#include "pch.h"
#include "RemoteTalkFoundation/Foundation.h"


class DSoundHandler : public DSoundHandlerBase
{
public:

} static g_dsound_handler;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        AddDSoundHandler(&g_dsound_handler);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
