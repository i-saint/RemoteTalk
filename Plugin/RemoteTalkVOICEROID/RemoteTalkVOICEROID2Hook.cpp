#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "rtvrCommon.h"
#include "rtvrHookHandler.h"
#include "rtvrServer.h"


rtvrITalkInterface* (*rtGetTalkInterface_)();
static bool rtcvLoadManagedModule()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkVOICEROID2Managed.dll";
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
            rt::AddWindowMessageHandler(&rtvrWindowMessageHandler::getInstance(), true, rt::HookType::Hotpatch);

            auto& dsound = rtvrDSoundHandler::getInstance();
            if (!rt::AddDSoundHandler(&dsound, true, rt::HookType::Hotpatch))
                rt::AddDSoundHandler(&dsound, true, rt::HookType::ATOverride);
            dsound.onPlay = []() { rtGetTalkInterface_()->onPlay(); };
            dsound.onStop = []() { rtGetTalkInterface_()->onStop(); };
            dsound.onUpdate = [](const rt::AudioData& ad) { rtGetTalkInterface_()->onUpdateBuffer(ad); };
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
