#include "pch.h"
#include "rtvrCommon.h"
#include "rtvr2InterfaceManaged.h"
#include "rtvr2Interface.h"


rtExport rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2TalkInterface::getInstance();
}
