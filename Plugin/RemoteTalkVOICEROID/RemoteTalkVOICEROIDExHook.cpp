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
        rt::AddWindowMessageHandler(&rtvrWindowMessageHandler::getInstance());

        bool ds_hooked = rt::InstallDSoundHook(rt::HookType::Hotpatch, false);
        if (!ds_hooked)
            ds_hooked = rt::InstallDSoundHook(rt::HookType::ATOverride);
        if (ds_hooked) {
            auto& dsound = rtvrDSoundHandler::getInstance();
            rt::AddDSoundHandler(&dsound);
            dsound.onPlay = []() {
            };
            dsound.onStop = []() {
                rtvrexTalkServer::getInstance().onStop();
            };
            dsound.onUpdate = [](const rt::AudioData& ad) {
                rtvrexTalkServer::getInstance().onUpdateSample(ad);
            };
        }
    }
    return TRUE;
}

