#pragma once
#include <atomic>
#include "RemoteTalk/RemoteTalk.h"

class rtcvITalkInterface : public rt::TalkInterface
{
public:
    virtual bool wait() = 0;

    virtual void onUpdateBuffer(rt::AudioData& ad) = 0;
#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};

extern rtcvITalkInterface* (*rtGetTalkInterface_)();
