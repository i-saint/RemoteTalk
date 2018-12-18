#pragma once
#include "RemoteTalk/RemoteTalk.h"

#define rtcvHostName "CeVIO CS6"
#define rtcvHookDll "RemoteTalkCeVIOCSHook.dll"
#define rtcvManagedDll "RemoteTalkCeVIOCSManaged.dll"
#define rtcvHostExe "CeVIO Creative Studio.exe"


class rtcvITalkInterface : public rt::TalkInterface
{
public:
    virtual bool wait() = 0;

#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};

extern rtcvITalkInterface* (*rtGetTalkInterface_)();
