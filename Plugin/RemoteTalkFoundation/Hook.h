#pragma once
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>


template<class T>
static inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

void* AllocExecutableForward(size_t size, void *location);
void* EmitJmpInstruction(void* from_, void* to_);
void* OverrideEAT(HMODULE module, const char *funcname, void *replacement, void *&jump_table);
void* OverrideIAT(HMODULE module, const char *target_module, const char *funcname, void *replacement);

void EnumerateModules(const std::function<void(HMODULE)>& body);
void EnumerateDLLImports(HMODULE module, const char *dllname, const std::function<void(const char*, void *&)> &body);
void EnumerateDLLExports(HMODULE module, const std::function<void(const char*, void *&)> &body);


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
#pragma warning(pop)

bool AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);
