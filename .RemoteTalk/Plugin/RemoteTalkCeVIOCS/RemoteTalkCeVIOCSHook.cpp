#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvHookHandler.h"
#include "rtcvTalkServer.h"


rtcv::ITalkInterface* (*rtGetTalkInterface_)();
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
            rt::AddWindowMessageHandler(&rtcv::WindowMessageHandler::getInstance());
            if (rt::InstallWaveOutHook(rt::HookType::Hotpatch)) {
                auto& wo = rtcv::WaveOutHandler::getInstance();
                rt::AddWaveOutHandler(&wo);
                wo.onUpdate = [](rt::AudioData& ad) {
                    rtcv::TalkServer::getInstance().onUpdateBuffer(ad);
                };
            }
        }
    }
    return TRUE;
}

rtAPI rt::TalkInterface* rtGetTalkInterface()
{
    return rtGetTalkInterface_ ? rtGetTalkInterface_() : nullptr;
}

rtAPI void rtOnManagedModuleUnload()
{
    rtcv::WaveOutHandler::getInstance().clearCallbacks();
}
