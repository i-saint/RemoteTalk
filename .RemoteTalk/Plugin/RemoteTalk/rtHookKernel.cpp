#include "pch.h"
#include "rtFoundation.h"
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtRawVector.h"
#ifdef _WIN32

namespace rt {

#pragma region LoadLibraryHandler
using LoadLibraryHandlers = FixedVector<LoadLibraryHandlerBase*, MaxHookChain>;
static LoadLibraryHandlers& GetLoadLibraryHandlers()
{
    static LoadLibraryHandlers s_handlers;
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
using CoCreateHandlers = FixedVector<CoCreateHandlerBase*, MaxHookChain>;
static CoCreateHandlers& GetCoCreateHandlers()
{
    static CoCreateHandlers s_handlers;
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
using WindowMessageHandlers = FixedVector<WindowMessageHandlerBase*, MaxHookChain>;
static WindowMessageHandlers& GetWindowMessageHandlers()
{
    static WindowMessageHandlers s_handlers;
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

} // namespace rt
#endif // _WIN32
