#pragma once
#ifdef _WIN32

namespace rt {

class LoadLibraryHandlerBase
{
public:
    LoadLibraryHandlerBase *prev = nullptr;

    virtual ~LoadLibraryHandlerBase() {}

    virtual void onLoadLibraryA(LPCSTR& lpFileName, HMODULE& ret) { prev->onLoadLibraryA(lpFileName, ret); }
    virtual void onLoadLibraryW(LPWSTR& lpFileName, HMODULE& ret) { prev->onLoadLibraryW(lpFileName, ret); }
    virtual void onLoadLibraryExA(LPCSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) { prev->onLoadLibraryExA(lpFileName, hFile, dwFlags, ret); }
    virtual void onLoadLibraryExW(LPWSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) { prev->onLoadLibraryExW(lpFileName, hFile, dwFlags, ret); }

    virtual void onLoadLibrary(HMODULE& ret) { prev->onLoadLibrary(ret); }
    virtual void onFreeLibrary(HMODULE& mod, BOOL& ret) { prev->onFreeLibrary(mod, ret); }
    virtual void onGetProcAddress(HMODULE& hModule, LPCSTR& lpProcName, FARPROC& ret) { prev->onGetProcAddress(hModule, lpProcName, ret); }
};

class CoCreateHandlerBase
{
public:
    CoCreateHandlerBase *prev = nullptr;

    virtual ~CoCreateHandlerBase() {}
    virtual void onCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, REFIID riid, LPVOID *&ppv, HRESULT& ret) { prev->onCoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv, ret); }
    virtual void onCoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, COSERVERINFO *&pServerInfo, DWORD& dwCount, MULTI_QI *&pResults, HRESULT& ret) { prev->onCoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults, ret); }
};

class WindowMessageHandlerBase
{
public:
    WindowMessageHandlerBase *prev = nullptr;

    virtual ~WindowMessageHandlerBase() {}
    virtual void onGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) { prev->onGetMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret); }
    virtual void onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) { prev->onGetMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret); }
};

class FileIOHandlerBase
{
public:
    FileIOHandlerBase *prev = nullptr;

    virtual ~FileIOHandlerBase() {}
    virtual void onCreateFileA(LPCSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, HANDLE& ret) { prev->onCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, ret); }
    virtual void onCreateFileW(LPCWSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, HANDLE& ret) { prev->onCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, ret); }
    virtual void onCreateFile2(LPCWSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, DWORD& dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS& pCreateExParams, HANDLE& ret) { prev->onCreateFile2(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, pCreateExParams, ret); }
    virtual void onReadFile(HANDLE& hFile, LPVOID& lpBuffer, DWORD& nNumberOfBytesToRead, LPDWORD& lpNumberOfBytesRead, LPOVERLAPPED& lpOverlapped, BOOL& ret) { prev->onReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped, ret); }
    virtual void onReadFileEx(HANDLE& hFile, LPVOID& lpBuffer, DWORD& nNumberOfBytesToRead, LPOVERLAPPED& lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE& lpCompletionRoutine, BOOL& ret) { prev->onReadFileEx(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine, ret); }
    virtual void onWriteFile(HANDLE& hFile, LPCVOID& lpBuffer, DWORD& nNumberOfBytesToWrite, LPDWORD& lpNumberOfBytesWritten, LPOVERLAPPED& lpOverlapped, BOOL& ret) { prev->onWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped, ret); }
    virtual void onWriteFileEx(HANDLE& hFile, LPCVOID& lpBuffer, DWORD& nNumberOfBytesToWrite, LPOVERLAPPED& lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE& lpCompletionRoutine, BOOL& ret) { prev->onWriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine, ret); }
    virtual void onCloseHandle(HANDLE& hObject, BOOL& ret) { prev->onCloseHandle(hObject, ret); }
};


bool InstallLoadLibraryHook(HookType ht);
void AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);

bool InstallCoCreateHook(HookType ht, bool load_dll = true);
void AddCoCreateHandler(CoCreateHandlerBase *handler);

bool InstallWindowMessageHook(HookType ht, bool load_dll = true);
bool OverrideFileIOIAT(HMODULE mod);
void AddWindowMessageHandler(WindowMessageHandlerBase *handler);

bool InstallFileIOHook(HookType ht, bool load_dll = true);
bool OverrideFileIOIAT(HMODULE mod);
void AddFileIOHandler(FileIOHandlerBase *handler);

} // namespace rt
#endif // _WIN32
