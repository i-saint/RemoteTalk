#pragma once

#define rtDefSingleton(T) static T& getInstance() { static T s_inst; return s_inst; }

#ifdef rtDebug
    #define rtDebugSleep(N) ::Sleep(N)
#else
    #define rtDebugSleep(N) 
#endif

#ifdef _WIN32
    #define rtExport extern "C" __declspec(dllexport)
    #define rtImport extern "C"
#else
    #define rtExport extern "C" 
    #define rtImport extern "C" 
#endif
