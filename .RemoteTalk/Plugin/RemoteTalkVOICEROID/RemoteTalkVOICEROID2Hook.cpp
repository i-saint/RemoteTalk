#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "rtvr2Common.h"
#include "rtvr2HookHandler.h"
#include "rtvr2TalkServer.h"

rtvr2::ITalkInterface* (*rtGetTalkInterface_)();
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
            rt::AddWindowMessageHandler(&rtvr2::WindowMessageHandler::getInstance());

            bool ds_hooked = rt::InstallDSoundHook(rt::HookType::Hotpatch, false);
            if (!ds_hooked)
                ds_hooked = rt::InstallDSoundHook(rt::HookType::ATOverride);
            if (ds_hooked) {
                auto& dsound = rtvr2::DSoundHandler::getInstance();
                rt::AddDSoundHandler(&dsound);
                dsound.onPlay = []() {
                    rtGetTalkInterface_()->onPlay();
                };
                dsound.onStop = []() {
                    rtvr2::TalkServer::getInstance().onStop();
                    rtGetTalkInterface_()->onStop();
                };
                dsound.onUpdate = [](const rt::AudioData& ad) {
                    rtvr2::TalkServer::getInstance().onUpdateSample(ad);
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
    auto& dsound = rtvr2::DSoundHandler::getInstance();
    dsound.onPlay = {};
    dsound.onStop = {};
    dsound.onUpdate = {};
}
