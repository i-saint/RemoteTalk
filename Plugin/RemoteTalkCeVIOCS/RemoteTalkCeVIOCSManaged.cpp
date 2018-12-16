#include "pch.h"
#include "rtcvCommon.h"
#include "rtcvInterfaceManaged.h"
#include "rtcvInterface.h"


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtcvTalkInterface::getInstance();
}
