#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"
#include "RemoteTalk/RemoteTalkNet.h"
#include "RemoteTalkVOICEROID2Controller.h"


class rtvr2DSoundHandler : public rt::DSoundHandlerBase
{
public:
    rtDefSingleton(rtvr2DSoundHandler);
    void afterCCIDirectSound8(LPDIRECTSOUND8 *&ppDS8, HRESULT& ret) override;
};

class rtvr2WindowMessageHandler : public rt::WindowMessageHandlerBase
{
public:
    rtDefSingleton(rtvr2WindowMessageHandler);
    void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override;
};

class rtvr2TalkServer : public rt::TalkServer
{
public:
    rtDefSingleton(rtvr2TalkServer);
    bool onSetParam(const std::string& name, const std::string& value) override;
    bool onTalk(const std::string& text) override;
};


rtvr2IController* (*rtvr2GetController)();

static bool rtvr2LoadController()
{
    auto path = rt::GetCurrentModuleDirectory() + "\\RemoteTalkVOICEROID2Managed.dll";
    auto mod = ::LoadLibraryA(path.c_str());
    if (!mod)
        return false;

    (void*&)rtvr2GetController = ::GetProcAddress(mod, "rtvr2GetController");
    return rtvr2GetController;
}


void rtvr2DSoundHandler::afterCCIDirectSound8(LPDIRECTSOUND8 *& ppDS8, HRESULT & ret)
{
}

void rtvr2WindowMessageHandler::afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret)
{
    auto& server = rtvr2TalkServer::getInstance();
    server.start();
    server.processMessages();
}

bool rtvr2TalkServer::onSetParam(const std::string & name, const std::string & value)
{
    return false;
}

bool rtvr2TalkServer::onTalk(const std::string & text)
{
    rtvr2GetController()->talk(text);
    return true;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (rtvr2LoadController()) {
            rt::AddDSoundHandler(&rtvr2DSoundHandler::getInstance());
            rt::AddWindowMessageHandler(&rtvr2WindowMessageHandler::getInstance());
        }
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return TRUE;
}
