#include "pch.h"
#include <atlbase.h>
#include "RemoteTalk/RemoteTalkNet.h"
#include "rtcvCommon.h"


#import "libid:D3AEA482-B527-4818-8CEA-810AFFCB24B6" named_guids rename_namespace("CeVIO")

static bool InjectDLL(HANDLE hProcess, const std::string& dllname)
{
    auto remote_addr = ::VirtualAllocEx(hProcess, 0, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (remote_addr == nullptr)
        return false;

    SIZE_T bytesRet = 0;
    size_t len = dllname.size() + 1;
    ::WriteProcessMemory(hProcess, remote_addr, dllname.c_str(), len, &bytesRet);

    auto hThread = ::CreateRemoteThread(hProcess, nullptr, 0, (LPTHREAD_START_ROUTINE)((void*)&LoadLibraryA), remote_addr, 0, nullptr);
    ::WaitForSingleObject(hThread, INFINITE);
    ::VirtualFreeEx(hProcess, remote_addr, 0, MEM_RELEASE);
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR cmd, int show)
{
    std::string module_dir = rt::GetCurrentModuleDirectory();
    std::string hook_path = module_dir + "\\" + rtcvHookDll;
    std::string config_path = module_dir + "\\" + rtcvConfigFile;

    rt::TalkServerSettings settings;
    settings.port = rtcvDefaultPort;

    auto try_launch = [&hook_path, &config_path, &settings](const std::string& exe_path) -> bool {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ::ZeroMemory(&si, sizeof(si));
        ::ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        BOOL ret = ::CreateProcessA(exe_path.c_str(), nullptr, nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);
        if (ret) {
            settings = rt::GetOrAddServerSettings(config_path, exe_path, rtcvDefaultPort);

            rtDebugSleep(7000); // for debug
            InjectDLL(pi.hProcess, hook_path);
            ::ResumeThread(pi.hThread);
            return true;
        }
        return false;
    };

    auto try_inject = [&hook_path, &config_path, &settings]() -> bool {
        auto proc = rt::FindProcess(rtcvHostExe);
        if (proc) {
            std::string exe_path = rt::GetMainModulePath(proc);
            settings = rt::GetOrAddServerSettings(config_path, exe_path, rtcvDefaultPort);

            if (InjectDLL(proc, hook_path)) {
                ::CloseHandle(proc);
                return true;
            }
        }
        return false;
    };

    auto wait_and_return = [&settings]() {
        rt::WaitUntilServerRespond(settings.port, 5000);
        return settings.port;
    };

    if (__argc > 1) {
        std::string exe_path = __argv[1];
        if (try_launch(exe_path))
            return wait_and_return();
    }
    else {
        if (try_inject())
            return wait_and_return();

        ::CoInitialize(NULL);
        CComPtr<CeVIO::IServiceControl> pServiceControl;
        HRESULT hr = ::CoCreateInstance(CeVIO::CLSID_ServiceControl, NULL, CLSCTX_INPROC_SERVER, CeVIO::IID_IServiceControl, reinterpret_cast<LPVOID *>(&pServiceControl));
        if (SUCCEEDED(hr)) {
            pServiceControl->StartHost(false);
            if (try_inject())
                return wait_and_return();
        }
    }

    return -1;
}
