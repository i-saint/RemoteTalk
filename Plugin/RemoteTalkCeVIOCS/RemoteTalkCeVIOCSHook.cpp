#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"


rtcvITalkInterface* (*rtGetTalkInterface_)();
static bool rtcvLoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\" rtcvManagedDll;
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, rtInterfaceFuncName);
    return rtGetTalkInterface_;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (rtcvLoadManagedModule()) {
            rt::InstallWindowMessageHook(rt::HookType::Hotpatch);
            rt::AddWindowMessageHandler(&rtcvWindowMessageHandler::getInstance());
            if (rt::InstallWaveOutHook(rt::HookType::Hotpatch)) {
                auto& wo = rtcvWaveOutHandler::getInstance();
                rt::AddWaveOutHandler(&wo);
                wo.onUpdate = [](rt::AudioData& ad) { rtGetTalkInterface_()->onUpdateBuffer(ad); };
            }
        }
    }
    return TRUE;
}

rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return rtGetTalkInterface_ ? rtGetTalkInterface_() : nullptr;
}

rtExport void rtOnManagedModuleUnload()
{
    rtcvWaveOutHandler::getInstance().clearCallbacks();
}
