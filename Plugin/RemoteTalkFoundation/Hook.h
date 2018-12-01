#pragma once
#include <windows.h>

template<class T>
static inline void ForceWrite(T &dst, const T &src)
{
    DWORD old_flag;
    ::VirtualProtect(&dst, sizeof(T), PAGE_EXECUTE_READWRITE, &old_flag);
    dst = src;
    ::VirtualProtect(&dst, sizeof(T), old_flag, &old_flag);
}

void* AllocateExecutableMemoryForward(size_t size, void *location);
void* EmitJmpInstruction(void* from_, void* to_);
void* OverrideDLLExport(HMODULE module, const char *funcname, void *replacement, void *&jump_table);
void* OverrideDLLImport(HMODULE module, const char *target_module, const char *funcname, void *replacement);
