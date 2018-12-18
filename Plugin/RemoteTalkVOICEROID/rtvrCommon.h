#pragma once
#include "RemoteTalk/RemoteTalk.h"

#define rtvr2HostName "VOICEROID2"
#define rtvr2HookDll "RemoteTalkVOICEROID2Hook.dll"
#define rtvr2ManagedDll "RemoteTalkVOICEROID2Managed.dll"
#define rtvr2HostExe "VoiceroidEditor.exe"


class rtvrITalkInterface : public rt::TalkInterface
{
public:
    virtual bool setCast(int v) = 0;

    virtual bool prepareUI() = 0;
    virtual void onPlay() = 0;
    virtual void onStop() = 0;
#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};

extern rtvrITalkInterface* (*rtGetTalkInterface_)();
