#pragma once
#include <atomic>
#include "RemoteTalk/rtFoundation.h"
#include "RemoteTalk/rtAudioData.h"
#include "RemoteTalk/rtTalkInterfaceImpl.h"
#include "RemoteTalk/rtSerialization.h"

class rtvrITalkInterface : public rt::TalkInterface
{
public:
    virtual bool setCast(int v) = 0;

    virtual bool prepareUI() = 0;
    virtual void onPlay() = 0;
    virtual void onStop() = 0;
    virtual void onUpdateBuffer(const rt::AudioData& ad) = 0;
#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};

extern rtvrITalkInterface* (*rtGetTalkInterface_)();
