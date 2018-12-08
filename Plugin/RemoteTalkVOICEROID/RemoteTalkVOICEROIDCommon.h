#pragma once
#include <string>
#include "RemoteTalk/rtTalkInterface.h"

class rtvr2TalkInterface : public rt::TalkInterface
{
public:
    virtual bool prepareUI() = 0;
    virtual void onPlay() = 0;
    virtual void onStop() = 0;
    virtual void onUpdateBuffer(const rt::AudioData& ad) = 0;
#ifdef rtDebug
    virtual bool onDebug() = 0;
#endif
};
