#include "pch.h"
#include "rtvr2Common.h"
#include "rtvr2TalkInterfaceManaged.h"
#include "rtvr2TalkInterface.h"


rtAPI rt::TalkInterface* rtGetTalkInterface()
{
    return &rtvr2::TalkInterface::getInstance();
}
