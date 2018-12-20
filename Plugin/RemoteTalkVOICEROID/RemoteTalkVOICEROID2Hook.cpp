#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "rtvr2Common.h"
#include "rtvr2HookHandler.h"
#include "rtvr2TalkServer.h"

rtvrITalkInterface* (*rtGetTalkInterface_)();
static bool rtvr2LoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\" rtvr2ManagedDll;
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtGetTalkInterface_ = ::GetProcAddress(mod, rtInterfaceFuncName);
    return rtGetTalkInterface_;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (rtvr2LoadManagedModule()) {
            rt::InstallWindowMessageHook(rt::HookType::Hotpatch);
            rt::AddWindowMessageHandler(&rtvrWindowMessageHandler::getInstance());

            bool ds_hooked = rt::InstallDSoundHook(rt::HookType::Hotpatch, false);
            if (!ds_hooked)
                ds_hooked = rt::InstallDSoundHook(rt::HookType::ATOverride);
            if (ds_hooked) {
                auto& dsound = rtvrDSoundHandler::getInstance();
                rt::AddDSoundHandler(&dsound);
                dsound.onPlay = []() {
                    rtGetTalkInterface_()->onPlay();
                };
                dsound.onStop = []() {
                    rtvrTalkServer::getInstance().onStop();
                    rtGetTalkInterface_()->onStop();
                };
                dsound.onUpdate = [](const rt::AudioData& ad) {
                    rtvrTalkServer::getInstance().onUpdateSample(ad);
                };
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
    auto& dsound = rtvrDSoundHandler::getInstance();
    dsound.onPlay = {};
    dsound.onStop = {};
    dsound.onUpdate = {};
}
