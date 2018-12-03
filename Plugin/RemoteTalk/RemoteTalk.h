#pragma once
#include "rtHook.h"
#include "rtHookKernel.h"
#include "rtHookWave.h"
#include "rtHookDSound.h"

#include "rtNorm.h"
#include "rtAudioData.h"

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
