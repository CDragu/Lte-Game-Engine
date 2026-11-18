#ifndef TOWER_PSP_RUNTIME_H
#define TOWER_PSP_RUNTIME_H

// Minimal PSP process setup used by the game entry point.

#define PSP_BOOST_CPU
// Undefine this if you don't want to
// overclock your cpu to 333MHz

#include <pspkernel.h>
#ifdef PSP_BOOST_CPU
#include <psppower.h>
#endif

#ifdef PSP_ENABLE_DEBUG
#include <pspdebug.h>
#define printf pspDebugScreenPrintf
#endif


static int exitCallback(int /*arg1*/, int /*arg2*/, void* /*common*/)
{
    sceKernelExitGame();
    return 0;
}

static int callbackThread(SceSize /*args*/, void* /*argp*/)
{
    const int cbid=sceKernelCreateCallback("Exit Callback",exitCallback,NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

static int setupCallbacks()
{
    const int thid=sceKernelCreateThread(
        "update_thread",callbackThread,0x11,0xFA0,0,0);
    if(thid>=0) sceKernelStartThread(thid,0,0);
    return thid;
}

static void setupPSP()
{
#ifdef PSP_BOOST_CPU
    scePowerSetClockFrequency(333, 333, 166);
#endif

    setupCallbacks();

#ifdef PSP_ENABLE_DEBUG
    pspDebugScreenInit();
#endif
}

#endif
