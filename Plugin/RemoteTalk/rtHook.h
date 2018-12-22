#pragma once
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <functional>


namespace rt {

template<class T>
static inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

bool IsValidMemory(const void *p);
bool IsValidModule(HMODULE module);
HMODULE GetModuleByAddr(const void *p);
std::string GetModuleDirectory(HMODULE module);
std::string GetCurrentModuleDirectory();
std::string GetMainModulePath(HANDLE proc);
std::string GetMainModulePath();
std::string GetMainModuleDirectory(HANDLE proc);
std::string GetMainModuleDirectory();

void* AllocExecutable(size_t size, void *location);
void* EmitJmpInstruction(void* from_, void* to_);
void* OverrideEAT(HMODULE module, const char *funcname, void *replacement, void *&jump_table);
void* OverrideIAT(HMODULE module, const char *target_module, const char *funcname, void *replacement);
void* Hotpatch(void *target, const void *replacement);

void EnumerateModules(HANDLE process, const std::function<void(HMODULE)>& body);
void EnumerateModules(const std::function<void(HMODULE)>& body);
void EnumerateDLLImports(HMODULE module, const char *dllname, const std::function<void(const char*, void *&)> &body);
void EnumerateDLLExports(HMODULE module, const std::function<void(const char*, void *&)> &body);

// caller must call CloseHandle() for returned handle
HANDLE FindProcess(const char *exe);

void EnumerateTopWindows(const std::function<void(HWND)> &body);
void EnumerateChildWindows(HWND parent, const std::function<void(HWND)> &body);
void EnumerateChildWindowsRecursive(HWND parent, const std::function<void(HWND)> &body);
void EnumerateAllWindows(const std::function<void(HWND)> &body);


enum class HookType
{
    ATOverride,
    Hotpatch,
};
} // namespace rt
#endif // _WIN32
