#pragma once

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
#pragma warning(pop)

bool AddLoadLibraryHandler(LoadLibraryHandlerBase *handler);
bool AddCoCreateHandler(CoCreateHandlerBase *handler, bool load_dll = true);
