#include "pch.h"
#include "RemoteTalk/RemoteTalk.h"

#define HookDllName "RemoteTalkVOICEROID2Hook.dll"
#define TargetExeName "VoiceroidEditor.exe"

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
    std::string module_path;
    {
        char buf[2048];
        ::GetModuleFileNameA(nullptr, buf, sizeof(buf));
        module_path = buf;
        auto spos = module_path.find_last_of("\\");
        if (spos != std::string::npos) {
            module_path.resize(spos);
        }
    }
    std::string hook_path = module_path + "\\" + HookDllName;

    std::string exe_path;

    if (__argc > 1) {
        // use arg if provided
        exe_path = __argv[1];
    }
    else {
        auto proc = rt::FindProcess(TargetExeName);
        if (proc) {
            InjectDLL(proc, hook_path);
            ::CloseHandle(proc);
            return 0;
        }
        else {
            if (exe_path.empty()) {
                // try to get install dir from registry
                char tmp[MAX_PATH + 1];
                DWORD tmp_size = sizeof(tmp);
                if (::RegGetValueA(HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{6F962085-923C-4AEE-BFC2-D64FAEE9B82D}",
                    "InstallLocation", RRF_RT_REG_SZ, NULL, tmp, &tmp_size) == ERROR_SUCCESS)
                {
                    exe_path = std::string(tmp, tmp_size - 1);
                    exe_path += TargetExeName;
                }
            }

            if (exe_path.empty()) {
                // try to open .exe in main module's dir
                exe_path = module_path + "\\" + TargetExeName;
            }
        }
    }

    {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ::ZeroMemory(&si, sizeof(si));
        ::ZeroMemory(&pi, sizeof(pi));
        si.cb = sizeof(si);
        BOOL ret = ::CreateProcessA(exe_path.c_str(), nullptr, nullptr, nullptr, FALSE,
            NORMAL_PRIORITY_CLASS | CREATE_SUSPENDED, nullptr, nullptr, &si, &pi);
        if (ret) {

            rtDebugSleep(7000); // for debug
            InjectDLL(pi.hProcess, hook_path);
            ::ResumeThread(pi.hThread);
            return 0;
        }
    }
    return 1;
}
