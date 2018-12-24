#include "pch.h"
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#ifdef _WIN32

namespace rt {

#pragma region LoadLibraryHandler
static std::vector<LoadLibraryHandlerBase*>& GetLoadLibraryHandlers()
{
    static std::vector<LoadLibraryHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetLoadLibraryHandlers().back()->Name(__VA_ARGS__);

static void OverrideLoadLibrary(HMODULE mod);

static HMODULE(WINAPI *LoadLibraryA_orig)(LPCSTR lpFileName);
static HMODULE WINAPI LoadLibraryA_hook(LPCSTR lpFileName)
{
    HMODULE ret = nullptr;
    Call(onLoadLibraryA, lpFileName, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryW_orig)(LPWSTR lpFileName);
static HMODULE WINAPI LoadLibraryW_hook(LPWSTR lpFileName)
{
    HMODULE ret = nullptr;
    Call(onLoadLibraryW, lpFileName, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryExA_orig)(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
static HMODULE WINAPI LoadLibraryExA_hook(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE ret = nullptr;
    Call(onLoadLibraryExA, lpFileName, hFile, dwFlags, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryExW_orig)(LPWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
static HMODULE WINAPI LoadLibraryExW_hook(LPWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    HMODULE ret = nullptr;
    Call(onLoadLibraryExW, lpFileName, hFile, dwFlags, ret);
    return ret;
}

static BOOL(WINAPI *FreeLibrary_orig)(HMODULE hLibModule);
static BOOL WINAPI FreeLibrary_hook(HMODULE hLibModule)
{
    BOOL ret = 0;
    Call(onFreeLibrary, hLibModule, ret);
    return ret;
}

static FARPROC(WINAPI *GetProcAddress_orig)(HMODULE hModule, LPCSTR lpProcName);
static FARPROC WINAPI GetProcAddress_hook(HMODULE hModule, LPCSTR lpProcName)
{
    auto ret = GetProcAddress_orig(hModule, lpProcName);
    Call(onGetProcAddress, hModule, lpProcName, ret);
    return ret;
}

class LoadLibraryHandlerRoot :public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandlerRoot);
    LoadLibraryHandlerRoot() { GetLoadLibraryHandlers().push_back(this); }

    void onLoadLibraryA(LPCSTR& lpFileName, HMODULE& ret) override
    {
        ret = LoadLibraryA_orig(lpFileName);
        Call(onLoadLibrary, ret);
    }
    void onLoadLibraryW(LPWSTR& lpFileName, HMODULE& ret) override
    {
        ret = LoadLibraryW_orig(lpFileName);
        Call(onLoadLibrary, ret);
    }
    void onLoadLibraryExA(LPCSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) override
    {
        ret = LoadLibraryExA_orig(lpFileName, hFile, dwFlags);
        Call(onLoadLibrary, ret);
    }
    void onLoadLibraryExW(LPWSTR& lpFileName, HANDLE& hFile, DWORD& dwFlags, HMODULE& ret) override
    {
        ret = LoadLibraryExW_orig(lpFileName, hFile, dwFlags);
        Call(onLoadLibrary, ret);
    }

    void onLoadLibrary(HMODULE& ret) override
    {
        OverrideLoadLibrary(ret);
    }
    void onFreeLibrary(HMODULE& mod, BOOL& ret) override
    {
        ret = FreeLibrary_orig(mod);
    }
    void onGetProcAddress(HMODULE& hModule, LPCSTR& lpProcName, FARPROC& ret) override
    {
        ret = GetProcAddress_orig(hModule, lpProcName);
    }
};
#undef Call

#define EachFunctions(Body)\
    Body(LoadLibraryA);\
    Body(LoadLibraryW);\
    Body(LoadLibraryExA);\
    Body(LoadLibraryExW);\
    Body(FreeLibrary);\
    Body(GetProcAddress);

static const char Kernel32_Dll[] = "kernel32.dll";

static void OverrideLoadLibrary(HMODULE mod)
{
    if (!IsValidModule(mod) || mod == GetModuleByAddr(&OverrideLoadLibrary))
        return;
#define Override(Name) OverrideIAT(mod, Kernel32_Dll, #Name, Name##_hook)
    EachFunctions(Override);
#undef Override
}

bool InstallLoadLibraryHook(HookType ht)
{
    auto mod = ::GetModuleHandleA(Kernel32_Dll);
    if (!mod)
        mod = ::LoadLibraryA(Kernel32_Dll);
    if (!mod)
        return false;

    void *tmp;
    if (ht == HookType::ATOverride) {
#define GetProc(Name) if(!Name##_orig){ (void*&)Name##_orig=::GetProcAddress(mod, #Name); }
        EachFunctions(GetProc);
#undef GetProc
        EnumerateModules([](HMODULE mod) { OverrideLoadLibrary(mod); });
    }
    else if (ht == HookType::Hotpatch) {
#define Override(Name) tmp=Hotpatch(::GetProcAddress(mod, #Name), Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override
    }
    return true;
}

void AddLoadLibraryHandler(LoadLibraryHandlerBase *handler)
{
    LoadLibraryHandlerRoot::getInstance();
    auto& handlers = GetLoadLibraryHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

#undef EachFunctions
#pragma endregion



#pragma region CoCreateHandler
static std::vector<CoCreateHandlerBase*>& GetCoCreateHandlers()
{
    static std::vector<CoCreateHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetCoCreateHandlers().back()->Name(__VA_ARGS__);

static HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    HRESULT ret = 0;
    Call(onCoCreateInstance, rclsid, pUnkOuter, dwClsContext, riid, ppv, ret);
    return ret;
}

static HRESULT WINAPI CoCreateInstanceEx_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults)
{
    HRESULT ret = 0;
    Call(onCoCreateInstanceEx, rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults, ret);
    return ret;
}
#undef Call


class CoCreateHandlerRoot : public CoCreateHandlerBase
{
public:
    rtDefSingleton(CoCreateHandlerRoot);
    CoCreateHandlerRoot() { GetCoCreateHandlers().push_back(this); }

    void onCoCreateInstance(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, REFIID riid, LPVOID *&ppv, HRESULT& ret) override
    {
        ret = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    }
    void onCoCreateInstanceEx(REFCLSID rclsid, LPUNKNOWN& pUnkOuter, DWORD& dwClsContext, COSERVERINFO *&pServerInfo, DWORD& dwCount, MULTI_QI *&pResults, HRESULT& ret) override
    {
        ret = CoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults);
    }
};


#define EachFunctions(Body)\
    Body(CoCreateInstance);\
    Body(CoCreateInstanceEx);

static const char OLE32_Dll[] = "ole32.dll";

class LoadLibraryHandler_CoCreate : public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandler_CoCreate);

    void onLoadLibrary(HMODULE& mod) override
    {
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
        if (!IsValidModule(mod) || mod == GetModuleByAddr(&hook))
            return;
#define Override(Name) OverrideIAT(mod, OLE32_Dll, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
};

bool InstallCoCreateHook(HookType ht, bool load_dll)
{
    auto mod = ::GetModuleHandleA(OLE32_Dll);
    if (!mod && load_dll)
        mod = ::LoadLibraryA(OLE32_Dll);
    if (!mod)
        return false;

    if (ht == HookType::ATOverride) {
        auto jumptable = AllocExecutable(1024, mod);
#define Override(Name) OverrideEAT(mod, #Name, Name##_hook, jumptable)
        EachFunctions(Override);
#undef Override

        InstallLoadLibraryHook(HookType::ATOverride);
        AddLoadLibraryHandler(&LoadLibraryHandler_CoCreate::getInstance());

        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_CoCreate::hook(mod); });
    }
    else if (ht == HookType::Hotpatch) {
        // ole32 seems incompatible with hotpatch...
    }
    return true;
}

void AddCoCreateHandler(CoCreateHandlerBase *handler)
{
    CoCreateHandlerRoot::getInstance();
    auto& handlers = GetCoCreateHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

#undef EachFunctions
#pragma endregion



#pragma region WindowMessageHandler
static std::vector<WindowMessageHandlerBase*>& GetWindowMessageHandlers()
{
    static std::vector<WindowMessageHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetWindowMessageHandlers().back()->Name(__VA_ARGS__);

static BOOL(WINAPI *GetMessageA_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
static BOOL WINAPI GetMessageA_hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    BOOL ret = 0;
    Call(onGetMessageA, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret);
    return ret;
}

static BOOL (WINAPI *GetMessageW_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
static BOOL WINAPI GetMessageW_hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    BOOL ret = 0;
    Call(onGetMessageW, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret);
    return ret;
}
#undef Call

class WindowMessageHandlerRoot : public WindowMessageHandlerBase
{
public:
    rtDefSingleton(WindowMessageHandlerRoot);
    WindowMessageHandlerRoot() { GetWindowMessageHandlers().push_back(this); }

    void onGetMessageA(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override
    {
        ret = GetMessageA_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    }
    void onGetMessageW(LPMSG& lpMsg, HWND& hWnd, UINT& wMsgFilterMin, UINT& wMsgFilterMax, BOOL& ret) override
    {
        ret = GetMessageW_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    }
};

#define EachFunctions(Body)\
    Body(GetMessageA);\
    Body(GetMessageW);

static const char User32_Dll[] = "user32.dll";

class LoadLibraryHandler_WindowMessage : public LoadLibraryHandlerBase
{
using super = LoadLibraryHandlerBase;
public:
    rtDefSingleton(LoadLibraryHandler_WindowMessage);

    void onLoadLibrary(HMODULE& mod) override
    {
        super::onLoadLibrary(mod);
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
#define Override(Name) OverrideIAT(mod, OLE32_Dll, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
};

bool InstallWindowMessageHook(HookType ht, bool load_dll)
{
    auto mod = ::GetModuleHandleA(User32_Dll);
    if (!mod && load_dll)
        mod = ::LoadLibraryA(User32_Dll);
    if (!mod)
        return false;

    void *tmp;
    if (ht == HookType::ATOverride) {
        auto jumptable = AllocExecutable(1024, mod);
#define Override(Name) tmp=OverrideEAT(mod, #Name, Name##_hook, jumptable); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override

        InstallLoadLibraryHook(HookType::ATOverride);
        AddLoadLibraryHandler(&LoadLibraryHandler_WindowMessage::getInstance());

        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_WindowMessage::hook(mod); });
    }
    else if (ht == HookType::Hotpatch) {
#define Override(Name) tmp=Hotpatch(::GetProcAddress(mod, #Name), Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override
    }
    return true;
}

void AddWindowMessageHandler(WindowMessageHandlerBase *handler)
{
    WindowMessageHandlerRoot::getInstance();
    auto& handlers = GetWindowMessageHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

#undef EachFunctions
#pragma endregion


#pragma region FileIOHandler
static std::vector<FileIOHandlerBase*>& GetFileIOHandlers()
{
    static std::vector<FileIOHandlerBase*> s_handlers;
    return s_handlers;
}
#define Call(Name, ...) GetFileIOHandlers().back()->Name(__VA_ARGS__);

HANDLE (WINAPI *CreateFileA_orig)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE WINAPI CreateFileA_hook(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    HANDLE ret = 0;
    Call(onCreateFileA, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, ret);
    return ret;
}

HANDLE (WINAPI *CreateFileW_orig)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile);
HANDLE WINAPI CreateFileW_hook(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    HANDLE ret = 0;
    Call(onCreateFileW, lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile, ret);
    return ret;
}

HANDLE (WINAPI *CreateFile2_orig)(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams);
HANDLE WINAPI CreateFile2_hook(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS pCreateExParams)
{
    HANDLE ret = 0;
    Call(onCreateFile2, lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, pCreateExParams, ret);
    return ret;
}

BOOL (WINAPI *ReadFile_orig)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
BOOL WINAPI ReadFile_hook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    BOOL ret = 0;
    Call(onReadFile, hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped, ret);
    return ret;
}

BOOL (WINAPI *ReadFileEx_orig)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
BOOL WINAPI ReadFileEx_hook(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    BOOL ret = 0;
    Call(onReadFileEx, hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine, ret);
    return ret;
}

BOOL (WINAPI *WriteFile_orig)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped);
BOOL WINAPI WriteFile_hook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
    BOOL ret = 0;
    Call(onWriteFile, hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped, ret);
    return ret;
}

BOOL (WINAPI *WriteFileEx_orig)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
BOOL WINAPI WriteFileEx_hook(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPOVERLAPPED lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    BOOL ret = 0;
    Call(onWriteFileEx, hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine, ret);
    return ret;
}

BOOL (WINAPI *CloseHandle_orig)(HANDLE hObject);
BOOL WINAPI CloseHandle_hook(HANDLE hObject)
{
    BOOL ret = 0;
    Call(onCloseHandle, hObject, ret);
    return ret;
}
#undef Call

class FileIOHandlerRoot : public FileIOHandlerBase
{
public:
    rtDefSingleton(FileIOHandlerRoot);
    FileIOHandlerRoot() { GetFileIOHandlers().push_back(this); }

    void onCreateFileA(LPCSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, HANDLE& ret) override
    {
        ret = CreateFileA_orig(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    void onCreateFileW(LPCWSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, LPSECURITY_ATTRIBUTES& lpSecurityAttributes, DWORD& dwCreationDisposition, DWORD& dwFlagsAndAttributes, HANDLE& hTemplateFile, HANDLE& ret) override
    {
        ret = CreateFileW_orig(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    void onCreateFile2(LPCWSTR& lpFileName, DWORD& dwDesiredAccess, DWORD& dwShareMode, DWORD& dwCreationDisposition, LPCREATEFILE2_EXTENDED_PARAMETERS& pCreateExParams, HANDLE& ret) override
    {
        ret = CreateFile2_orig(lpFileName, dwDesiredAccess, dwShareMode, dwCreationDisposition, pCreateExParams);
    }
    void onReadFile(HANDLE& hFile, LPVOID& lpBuffer, DWORD& nNumberOfBytesToRead, LPDWORD& lpNumberOfBytesRead, LPOVERLAPPED& lpOverlapped, BOOL& ret) override
    {
        ret = ReadFile_orig(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
    }
    void onReadFileEx(HANDLE& hFile, LPVOID& lpBuffer, DWORD& nNumberOfBytesToRead, LPOVERLAPPED& lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE& lpCompletionRoutine, BOOL& ret) override
    {
        ret = ReadFileEx_orig(hFile, lpBuffer, nNumberOfBytesToRead, lpOverlapped, lpCompletionRoutine);
    }
    void onWriteFile(HANDLE& hFile, LPCVOID& lpBuffer, DWORD& nNumberOfBytesToWrite, LPDWORD& lpNumberOfBytesWritten, LPOVERLAPPED& lpOverlapped, BOOL& ret) override
    {
        ret = WriteFile_orig(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
    }
    void onWriteFileEx(HANDLE& hFile, LPCVOID& lpBuffer, DWORD& nNumberOfBytesToWrite, LPOVERLAPPED& lpOverlapped, LPOVERLAPPED_COMPLETION_ROUTINE& lpCompletionRoutine, BOOL& ret) override
    {
        ret = WriteFileEx_orig(hFile, lpBuffer, nNumberOfBytesToWrite, lpOverlapped, lpCompletionRoutine);
    }
    void onCloseHandle(HANDLE& hObject, BOOL& ret) override
    {
        ret = CloseHandle_orig(hObject);
    }
};


#define EachFunctions(Body)\
    Body(CreateFileA);\
    Body(CreateFileW);\
    Body(CreateFile2);\
    Body(ReadFile);\
    Body(ReadFileEx);\
    Body(WriteFile);\
    Body(WriteFileEx);\
    Body(CloseHandle);

static const char KernelBase_Dll[] = "KernelBase.dll";
static const char CoreFile_Dll[] = "api-ms-win-core-file-l1-1-0.dll";

class LoadLibraryHandler_FileIO : public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandler_FileIO);

    void onLoadLibrary(HMODULE& mod) override
    {
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
        if (!IsValidModule(mod) || mod == ::GetModuleHandleA(Kernel32_Dll))
            return;
#define Override(Name) OverrideIAT(mod, Kernel32_Dll, #Name, Name##_hook); OverrideIAT(mod, CoreFile_Dll, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
};


bool InstallFileIOHook(HookType ht, bool load_dll)
{
    auto mod = ::GetModuleHandleA(Kernel32_Dll);
    if (!mod && load_dll)
        mod = ::LoadLibraryA(Kernel32_Dll);
    if (!mod)
        return false;

    void *tmp;
    if (ht == HookType::ATOverride) {
        auto jumptable = AllocExecutable(1024, mod);
#define Override(Name) tmp=OverrideEAT(mod, #Name, Name##_hook, jumptable); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override

        InstallLoadLibraryHook(HookType::ATOverride);
        AddLoadLibraryHandler(&LoadLibraryHandler_FileIO::getInstance());

        EnumerateModules([](HMODULE mod) { LoadLibraryHandler_FileIO::hook(mod); });
    }
    else if (ht == HookType::Hotpatch) {
        auto kb = ::GetModuleHandleA(KernelBase_Dll);
        if (kb != nullptr)
            mod = kb;

#define Override(Name) tmp=Hotpatch(::GetProcAddress(mod, #Name), Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
        EachFunctions(Override);
#undef Override
    }
    return true;
}

bool OverrideFileIOIAT(HMODULE mod)
{
    if (!mod)
        return false;

    void *tmp;
#define Override(Name) tmp=OverrideIAT(mod, Kernel32_Dll, #Name, Name##_hook); if(!Name##_orig){ (void*&)Name##_orig=tmp; }
    EachFunctions(Override);
#undef Override
    return true;
}

void AddFileIOHandler(FileIOHandlerBase *handler)
{
    FileIOHandlerRoot::getInstance();
    auto& handlers = GetFileIOHandlers();
    handler->prev = handlers.back();
    handlers.push_back(handler);
}

#undef EachFunctions
#pragma endregion

} // namespace rt
#endif // _WIN32
