#include "pch.h"
#include "rtFoundation.h"
#include "rtRawVector.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtHookFileIO.h"
#ifdef _WIN32

namespace rt {

using FileIOHandlers = FixedVector<FileIOHandlerBase*, MaxHookChain>;
static FileIOHandlers& GetFileIOHandlers()
{
    static FileIOHandlers s_handlers;
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

static const char Kernel32_Dll[] = "kernel32.dll";
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

} // namespace rt
#endif // _WIN32
