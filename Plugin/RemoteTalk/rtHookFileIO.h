#pragma once
#ifdef _WIN32

namespace rt {

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

bool InstallFileIOHook(HookType ht, bool load_dll = true);
bool OverrideFileIOIAT(HMODULE mod);
void AddFileIOHandler(FileIOHandlerBase *handler);

} // namespace rt
#endif // _WIN32
