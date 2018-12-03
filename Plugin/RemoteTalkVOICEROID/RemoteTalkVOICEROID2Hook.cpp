#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalkVOICEROID2Controller.h"

rtvr2IController* (*rtvr2GetController)();

static bool LoadController()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkVOICEROID2Managed.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtvr2GetController = ::GetProcAddress(mod, "rtvr2GetController");
    return rtvr2GetController;
}


class DSoundHandler : public rt::DSoundHandlerBase
{
public:
    void afterCCIDirectSound8(LPDIRECTSOUND8 *&ppDS8, HRESULT& ret) override;
} static g_dsound_handler;

class WindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    void afterGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL ret) override;
} g_wm_handler;

void DSoundHandler::afterCCIDirectSound8(LPDIRECTSOUND8 *& ppDS8, HRESULT & ret)
{
    printf("");
}


void WindowMessageHandler::afterGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, BOOL ret)
{
    static int s_count;
    ++s_count;
    if (s_count % 1000 == 0) {
        std::vector<std::string> windows;
        rtvr2GetController()->dbgListWindows(windows);

        rtvr2GetController()->talk("はろーぼいすろいど");
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (LoadController()) {
            rt::AddDSoundHandler(&g_dsound_handler);
            rt::AddWindowMessageHandler(&g_wm_handler);
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
