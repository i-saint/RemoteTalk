#pragma once
#ifdef _WIN32

namespace rt {

#pragma warning(push)
#pragma warning(disable:4100)
class LoadLibraryHandlerBase
{
public:
    virtual ~LoadLibraryHandlerBase() {}
    virtual void afterLoadLibrary(HMODULE& mod) {}
    virtual void beforeFreeLibrary(HMODULE& mod) {}
    virtual void afterGetProcAddress(HMODULE& hModule, LPCSTR& lpProcName, FARPROC& ret) {}

};

class CoCreateHandlerBase
{
public:
    virtual ~CoCreateHandlerBase() {}
    virtual void afterCoGetClassObject(REFCLSID rclsid, DWORD& dwClsContext, LPVOID& pvReserved, REFIID riid, LPVOID *&ppv, HRESULT& ret) {}
    virtual void afterCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, REFIID riid, LPVOID *&ppv, HRESULT& ret) {}
    virtual void afterCoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, COSERVERINFO *&pServerInfo, DWORD& dwCount, MULTI_QI *&pResults, HRESULT& ret) {}
};

class WindowMessageHandlerBase
{
public:
    virtual ~WindowMessageHandlerBase() {}
    virtual void beforeGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax) {}
    virtual void afterGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) {}
    virtual void beforeGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax) {}
    virtual void afterGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) {}
};
#pragma warning(pop)

bool InstallLoadLibraryHook(HookType ht);
void AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);

bool InstallCoCreateHook(HookType ht, bool load_dll = true);
void AddCoCreateHandler(CoCreateHandlerBase *handler);

bool InstallWindowMessageHook(HookType ht, bool load_dll = true);
void AddWindowMessageHandler(WindowMessageHandlerBase *handler);

} // namespace rt
#endif // _WIN32
