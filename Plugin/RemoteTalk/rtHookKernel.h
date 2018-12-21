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

class FileIOHandlerBase
{
public:
    virtual ~FileIOHandlerBase() {}
    virtual void afterCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, HANDLE ret) {}
    virtual void afterCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile, HANDLE ret) {}
    virtual void afterCreateFile2(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams, HANDLE ret) {}
    virtual void afterReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped, BOOL ret) {}
    virtual void afterReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, BOOL ret) {}
    virtual void afterWriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped, BOOL ret) {}
    virtual void afterWriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine, BOOL ret) {}
    virtual void beforeCloseHandle(HANDLE hObject) {}
};
#pragma warning(pop)

bool InstallLoadLibraryHook(HookType ht);
void AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);

bool InstallCoCreateHook(HookType ht, bool load_dll = true);
void AddCoCreateHandler(CoCreateHandlerBase *handler);

bool InstallWindowMessageHook(HookType ht, bool load_dll = true);
bool OverrideFileIOIAT(HMODULE mod);
void AddWindowMessageHandler(WindowMessageHandlerBase *handler);

} // namespace rt
#endif // _WIN32
