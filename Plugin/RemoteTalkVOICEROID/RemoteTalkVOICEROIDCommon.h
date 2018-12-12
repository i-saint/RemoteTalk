#pragma once
#include "RemoteTalk/rtTalkInterface.h"

class rtvr2ITalkInterface : public rt::TalkInterface
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
