#include <windows.h>
#include "exports.h"
#include <GlumityLib.h>

#define DELAY_TIME 5000

DWORD g_MainThreadId = 0;

EXPORT DWORD GetApplicationMainThreadID()
{
    return g_MainThreadId;
}

void Main()
{
    // Check if we are already running
    HANDLE hMutex = CreateMutexA(NULL, TRUE, "GlumityToolsuiteV2");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // We are already running, stop
        CloseHandle(hMutex);
        return;
    }
    // Load modloader

    // Wait some time to avoid threading issues
    Sleep(DELAY_TIME);
    LoadLibraryA(GLUMITYV2_DLL);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    HANDLE hThread = NULL;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        g_MainThreadId = GetCurrentThreadId();
        hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Main, 0, 0, 0);
        if (hThread)
            CloseHandle(hThread);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:

        break;
    }
    return TRUE;
}