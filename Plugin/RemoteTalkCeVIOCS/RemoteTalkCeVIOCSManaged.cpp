#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvTalkInterfaceManaged.h"
#include "rtcvTalkInterface.h"


rtAPI rt::TalkInterface* rtGetTalkInterface()
{
    return &rtcv::TalkInterface::getInstance();
}
