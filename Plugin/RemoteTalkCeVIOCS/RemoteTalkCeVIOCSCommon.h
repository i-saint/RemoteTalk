#pragma once
#include "RemoteTalk/rtTalkInterface.h"

class rtcvITalkInterface : public rt::TalkInterface
{
public:
    virtual bool wait() = 0;

    virtual void onPlay() = 0;
    virtual void onStop() = 0;
    virtual void onUpdateBuffer(rt::AudioData& ad) = 0;
#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};
