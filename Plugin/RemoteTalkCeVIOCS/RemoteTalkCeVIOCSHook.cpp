#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"


class WaveOutHandler : public rt::WaveOutHandlerBase
{
public:

} static g_waveout_handler;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        rt::AddWaveOutHandler(&g_waveout_handler);
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
