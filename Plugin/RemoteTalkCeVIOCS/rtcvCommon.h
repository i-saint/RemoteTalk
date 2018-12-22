#pragma once
#include "RemoteTalk/RemoteTalk.h"

#define rtcvHostName    "CeVIO CS"
#define rtcvHookDll     "RemoteTalkCeVIOCSHook.dll"
#define rtcvManagedDll  "RemoteTalkCeVIOCSManaged.dll"
#define rtcvHostExe     "CeVIO Creative Studio.exe"
#define rtcvConfigFile  "RemoteTalkCeVIOCS.json"
#define rtcvDefaultPort 8140

namespace rtcv {

class ITalkInterface : public rt::TalkInterface
{
public:
    virtual bool wait() = 0;

#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};

} // namespace rtcv

extern rtcv::ITalkInterface* (*rtGetTalkInterface_)();
