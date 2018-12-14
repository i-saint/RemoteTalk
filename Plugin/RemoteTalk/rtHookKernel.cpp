#include "pch.h"
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#ifdef _WIN32

namespace rt {

#pragma region Kernel32
static std::vector<LoadLibraryHandlerBase*> g_loadlibrary_handlers;
#define Call(Name, ...) for(auto *handler : g_loadlibrary_handlers) { handler->Name(__VA_ARGS__); }

static void OverrideLoadLibrary(HMODULE mod);

static HMODULE(WINAPI *LoadLibraryA_orig)(LPCSTR lpFileName);
static HMODULE WINAPI LoadLibraryA_hook(LPCSTR lpFileName)
{
    auto ret = LoadLibraryA_orig(lpFileName);
    OverrideLoadLibrary(ret);
    Call(afterLoadLibrary, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryW_orig)(LPWSTR lpFileName);
static HMODULE WINAPI LoadLibraryW_hook(LPWSTR lpFileName)
{
    auto ret = LoadLibraryW_orig(lpFileName);
    OverrideLoadLibrary(ret);
    Call(afterLoadLibrary, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryExA_orig)(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags);
static HMODULE WINAPI LoadLibraryExA_hook(LPCSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    auto ret = LoadLibraryExA_orig(lpFileName, hFile, dwFlags);
    OverrideLoadLibrary(ret);
    Call(afterLoadLibrary, ret);
    return ret;
}

static HMODULE(WINAPI *LoadLibraryExW_orig)(LPWSTR lpFileName, HANDLE hFile, DWORD dwFlags);
static HMODULE WINAPI LoadLibraryExW_hook(LPWSTR lpFileName, HANDLE hFile, DWORD dwFlags)
{
    auto ret = LoadLibraryExW_orig(lpFileName, hFile, dwFlags);
    OverrideLoadLibrary(ret);
    Call(afterLoadLibrary, ret);
    return ret;
}

static BOOL(WINAPI *FreeLibrary_orig)(HMODULE hLibModule);
static BOOL WINAPI FreeLibrary_hook(HMODULE hLibModule)
{
    Call(beforeFreeLibrary, hLibModule);
    auto ret = FreeLibrary_orig(hLibModule);
    return ret;
}

static FARPROC(WINAPI *GetProcAddress_orig)(HMODULE hModule, LPCSTR lpProcName);
static FARPROC WINAPI GetProcAddress_hook(HMODULE hModule, LPCSTR lpProcName)
{
    auto ret = GetProcAddress_orig(hModule, lpProcName);
    Call(afterGetProcAddress, hModule, lpProcName, ret);
    return ret;
}
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

bool AddLoadLibraryHandler(LoadLibraryHandlerBase *handler, HookType ht)
{
    static bool s_first = true;
    if (s_first) {
        s_first = false;

        auto mod = GetModuleHandleA(Kernel32_Dll);
        if (ht == HookType::ATOverride) {
#define GetProc(Name) (void*&)Name##_orig = GetProcAddress(mod, #Name)
            EachFunctions(GetProc);
#undef GetProc
            EnumerateModules([](HMODULE mod) { OverrideLoadLibrary(mod); });
        }
        else if (ht == HookType::Hotpatch)
        {
#define Override(Name) (void*&)Name##_orig = Hotpatch(::GetProcAddress(mod, #Name), Name##_hook)
            EachFunctions(Override);
#undef Override
        }
    }

    g_loadlibrary_handlers.push_back(handler);
    return true;
}

#undef EachFunctions
#pragma endregion



#pragma region OLE32
static std::vector<CoCreateHandlerBase*> g_cocreate_handlers;
#define Call(Name, ...) for(auto *handler : g_cocreate_handlers) { handler->Name(__VA_ARGS__); }

static HRESULT WINAPI CoGetClassObject_hook(REFCLSID rclsid, DWORD dwClsContext, LPVOID pvReserved, REFIID riid, LPVOID *ppv)
{
    auto ret = CoGetClassObject(rclsid, dwClsContext, pvReserved, riid, ppv);
    Call(afterCoGetClassObject, rclsid, dwClsContext, pvReserved, riid, ppv, ret);
    return ret;
}

static HRESULT WINAPI CoCreateInstance_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv)
{
    auto ret = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
    Call(afterCoCreateInstance, rclsid, pUnkOuter, dwClsContext, riid, ppv, ret);
    return ret;
}

static HRESULT WINAPI CoCreateInstanceEx_hook(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, COSERVERINFO *pServerInfo, DWORD dwCount, MULTI_QI *pResults)
{
    auto ret = CoCreateInstanceEx(rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults);
    Call(afterCoCreateInstanceEx, rclsid, pUnkOuter, dwClsContext, pServerInfo, dwCount, pResults, ret);
    return ret;
}
#undef Call

#define EachFunctions(Body)\
    Body(CoGetClassObject);\
    Body(CoCreateInstance);\
    Body(CoCreateInstanceEx);

static const char OLE32_Dll[] = "ole32.dll";

class LoadLibraryHandler_OLE32 : public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandler_OLE32);

    void afterLoadLibrary(HMODULE& mod) override
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


bool AddCoCreateHandler(CoCreateHandlerBase *handler, bool load_dll, HookType ht)
{
    // setup hooks
    auto ole32 = load_dll ? ::LoadLibraryA(OLE32_Dll) : ::GetModuleHandleA(OLE32_Dll);
    if (!ole32)
        return false;

    static bool s_first = true;
    if (s_first) {
        s_first = false;

        if (ht == HookType::ATOverride) {
            auto jumptable = AllocExecutableForward(1024, ole32);
#define Override(Name) OverrideEAT(ole32, #Name, Name##_hook, jumptable)
            EachFunctions(Override);
#undef Override

            AddLoadLibraryHandler(&LoadLibraryHandler_OLE32::getInstance());
            EnumerateModules([](HMODULE mod) { LoadLibraryHandler_OLE32::hook(mod); });
        }
        else if (ht == HookType::Hotpatch) {
        }
    }

    g_cocreate_handlers.push_back(handler);
    return true;
}

#undef EachFunctions
#pragma endregion



#pragma region User32
static std::vector<WindowMessageHandlerBase*> g_windowmessage_handlers;
#define Call(Name, ...) for(auto *handler : g_windowmessage_handlers) { handler->Name(__VA_ARGS__); }

static BOOL(WINAPI *GetMessageA_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
static BOOL WINAPI GetMessageA_hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    Call(beforeGetMessageA, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    auto ret = GetMessageA_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    Call(afterGetMessageA, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret);
    return ret;
}

static BOOL (WINAPI *GetMessageW_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
static BOOL WINAPI GetMessageW_hook(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
    Call(beforeGetMessageW, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    auto ret = GetMessageW_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
    Call(afterGetMessageW, lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, ret);
    return ret;
}
#undef Call

#define EachFunctions(Body)\
    Body(GetMessageA);\
    Body(GetMessageW);

static const char User32_Dll[] = "user32.dll";

class LoadLibraryHandler_User32 : public LoadLibraryHandlerBase
{
public:
    rtDefSingleton(LoadLibraryHandler_User32);

    void afterLoadLibrary(HMODULE& mod) override
    {
        hook(mod);
    }

    static void hook(HMODULE mod)
    {
#define Override(Name) OverrideIAT(mod, OLE32_Dll, #Name, Name##_hook)
        EachFunctions(Override);
#undef Override
    }
};

bool AddWindowMessageHandler(WindowMessageHandlerBase *handler, bool load_dll, HookType ht)
{
    // setup hooks
    auto mod = load_dll ? ::LoadLibraryA(User32_Dll) : ::GetModuleHandleA(User32_Dll);
    if (!mod)
        return false;

    static bool s_first = true;
    if (s_first) {
        s_first = false;

        if (ht == HookType::ATOverride) {
            auto jumptable = AllocExecutableForward(1024, mod);
#define Override(Name) (void*&)Name##_orig = OverrideEAT(mod, #Name, Name##_hook, jumptable)
            EachFunctions(Override);
#undef Override

            AddLoadLibraryHandler(&LoadLibraryHandler_User32::getInstance());
            EnumerateModules([](HMODULE mod) { LoadLibraryHandler_User32::hook(mod); });
        }
        else if (ht == HookType::Hotpatch) {
#define Override(Name) (void*&)Name##_orig = Hotpatch(::GetProcAddress(mod, #Name), Name##_hook)
            EachFunctions(Override);
#undef Override
        }
    }

    g_windowmessage_handlers.push_back(handler);
    return true;
}

#undef EachFunctions
#pragma endregion
} // namespace rt
#endif // _WIN32
