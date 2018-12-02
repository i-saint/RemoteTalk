#pragma once
#include "Hook.h"
#include "HookKernel.h"
#include "HookWave.h"
#include "HookDSound.h"

#include "Norm.h"
#include "AudioData.h"

#ifdef rtDebug
    #define rtDebugSleep(N) ::Sleep(N)
#else
    #define rtDebugSleep(N) 
#endif
