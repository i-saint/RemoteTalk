#pragma once
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
bool IsValidModule(HMODULE& module);
HMODULE GetModuleByAddr(const void *p);
void* AllocExecutableForward(size_t size, void *location);
void* EmitJmpInstruction(void* from_, void* to_);
void* OverrideEAT(HMODULE module, const char *funcname, void *replacement, void *&jump_table);
void* OverrideIAT(HMODULE module, const char *target_module, const char *funcname, void *replacement);
void* Hotpatch(void *target, const void *replacement);

void EnumerateModules(const std::function<void(HMODULE)>& body);
void EnumerateDLLImports(HMODULE module, const char *dllname, const std::function<void(const char*, void *&)> &body);
void EnumerateDLLExports(HMODULE module, const std::function<void(const char*, void *&)> &body);

} // namespace rt
