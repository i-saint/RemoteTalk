#pragma once

#define rtPluginVersion 20181226
#define rtPluginVersionStr "20181226"
#define rtProtocolVersion 100


#define rtDefSingleton(T) static T& getInstance() { static T s_inst; return s_inst; }

namespace rt {
    void Print(const char *fmt, ...);
    void Print(const wchar_t *fmt, ...);
} // namespace rt

#define rtLogInfo(...)    ::rt::Print("RemoteTalk info: " __VA_ARGS__)
#define rtLogWarning(...) ::rt::Print("RemoteTalk warning: " __VA_ARGS__)
#define rtLogError(...)   ::rt::Print("RemoteTalk error: " __VA_ARGS__)

#ifdef rtDebug
    #define rtLogDebug(...) ::rt::Print("RemoteTalk debug: " __VA_ARGS__)
    #define rtDebugSleep(N) ::Sleep(N)
#else
    #define rtLogDebug(...)
    #define rtDebugSleep(N) 
#endif

#ifdef _WIN32
    #define rtAPI extern "C" __declspec(dllexport)
#else
    #define rtAPI extern "C" 
#endif
