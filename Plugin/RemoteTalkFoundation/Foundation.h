#pragma once
#include "Hook.h"
#include "HookWave.h"
#include "HookDSound.h"

#ifdef rtDebug
    #define rtDebugSleep(N) ::Sleep(N)
#else
    #define rtDebugSleep(N) 
#endif
