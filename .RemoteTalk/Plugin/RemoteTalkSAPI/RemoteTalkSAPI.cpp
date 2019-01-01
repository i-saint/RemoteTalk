#include "pch.h"
#include "rtspCommon.h"
#include "rtspHookHandler.h"
#include "rtspTalkServer.h"


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (FAILED(::CoInitialize(NULL)))
            return FALSE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}

rtAPI rt::TalkInterface* rtGetTalkInterface()
{
    return nullptr;
}

rtAPI bool rtspIsServerRunning()
{
    return rtsp::TalkServer::getInstance().isRunning();
}

rtAPI int rtspGetServerPort()
{
    return rtsp::TalkServer::getInstance().getSettings().port;
}

rtAPI bool rtspStartServer(const char *config_path)
{
    auto& inst = rtsp::TalkServer::getInstance();
    if (!inst.isRunning()) {
        inst.loadConfig(config_path);
        return inst.start();
    }
    return false;
}

rtAPI bool rtspStopServer()
{
    auto& inst = rtsp::TalkServer::getInstance();
    if (inst.isRunning()) {
        inst.stop();
        return true;
    }
    return false;
}
