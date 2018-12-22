#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "rtvrexCommon.h"
#include "rtvrexHookHandler.h"
#include "rtvrexTalkServer.h"



BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        rt::InstallWindowMessageHook(rt::HookType::Hotpatch);
        rt::AddWindowMessageHandler(&rtvrex::WindowMessageHandler::getInstance());

        bool ds_hooked = rt::InstallDSoundHook(rt::HookType::Hotpatch, false);
        if (!ds_hooked)
            ds_hooked = rt::InstallDSoundHook(rt::HookType::ATOverride);
        if (ds_hooked)
            rt::AddDSoundHandler(&rtvrex::DSoundHandler::getInstance());
    }
    return TRUE;
}

