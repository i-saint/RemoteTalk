#include "pch.h"
#include "RemoteTalkFoundation/Foundation.h"


class WaveOutHandler : public WaveOutHandlerBase
{
public:

} static g_waveout_handler;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        AddWaveOutHandler(&g_waveout_handler);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
