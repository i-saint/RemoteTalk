#pragma once
#ifdef _WIN32

namespace rt {

class LoadLibraryHandlerBase
{
public:
    LoadLibraryHandlerBase *prev = nullptr;

    virtual ~LoadLibraryHandlerBase() {}

    virtual void onLoadLibraryA(LPCSTR& lpFileName, HMODULE& ret) { if (prev) prev->onLoadLibraryA(lpFileName, ret); }
    virtual void onLoadLibraryW(LPWSTR& lpFileName, HMODULE& ret) { if (prev) prev->onLoadLibraryW(lpFileName, ret); }
    virtual void onLoadLibraryExA(LPCSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) { if (prev) prev->onLoadLibraryExA(lpFileName, hFile, dwFlags, ret); }
    virtual void onLoadLibraryExW(LPWSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) { if (prev) prev->onLoadLibraryExW(lpFileName, hFile, dwFlags, ret); }

    virtual void onLoadLibrary(HMODULE& ret) { if (prev) prev->onLoadLibrary(ret); }
    virtual void onFreeLibrary(HMODULE& mod, BOOL& ret) { if (prev) prev->onFreeLibrary(mod, ret); }
    virtual void onGetProcAddress(HMODULE& hModule, LPCSTR& lpProcName, FARPROC& ret) { if (prev) prev->onGetProcAddress(hModule, lpProcName, ret); }
};

class CoCreateHandlerBase
{
public:
    CoCreateHandlerBase *prev = nullptr;

    virtual ~CoCreateHandlerBase() {}
    virtual void onCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, REFIID riid, LPVOID *&ppv, HRESULT& ret) { if (prev) prev->onCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv, ret); }
    virtual void onCoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, COSERVERINFO *&pServerInfo, DWORD& dwCount, MULTI_QI *&pResults, HRESULT& ret) { if (prev) prev->onCoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults, ret); }
};

class WindowMessageHandlerBase
{
public:
    WindowMessageHandlerBase *prev = nullptr;

    virtual ~WindowMessageHandlerBase() {}
    virtual void onGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) { if (prev) prev->onGetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret); }
    virtual void onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) { if (prev) prev->onGetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret); }
};


bool InstallLoadLibraryHook(HookType ht);
void AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);

bool InstallCoCreateHook(HookType ht, bool load_dll = true);
void AddCoCreateHandler(CoCreateHandlerBase *handler);

bool InstallWindowMessageHook(HookType ht, bool load_dll = true);
bool OverrideFileIOIAT(HMODULE mod);
void AddWindowMessageHandler(WindowMessageHandlerBase *handler);

} // namespace rt
#endif // _WIN32
