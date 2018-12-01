#include "pch.h"
#include "RemoteTalkFoundation/Foundation.h"


class WaveOutHandler : public WaveOutHandlerBase
{
public:

} g_waveout_handler;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        // setup hooks
        auto winmm = ::LoadLibraryA("winmm.dll");
        HookWaveOutFunctions(&g_waveout_handler);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
