#ifndef GLUMITY_HOOKING_H
#define GLUMITY_HOOKING_H

#include <windows.h>
#include <stdio.h>
#include <MinHook.h>

void GlumityPlugin_printf(const char *fmt, const char *printHeaderInner, ...);

inline void GlumityV2_MakeHookAtomic(void **original_ptr, void *hook_ptr)
{
#if defined(_MSC_VER)
    InterlockedCompareExchangePointer(original_ptr, hook_ptr, NULL);
#else
    void *expected = NULL;
    __asm__ __volatile__(
        "lock cmpxchg %2, %0"
        : "+m"(*original_ptr), "=a"(expected)
        : "r"(hook_ptr), "a"(expected)
        : "cc");
#endif
}

#define GLUMITYV2_INIT_HOOKING(pluginName, onFail)                                                      \
    MH_STATUS stat = MH_Initialize();                                                                   \
    if (stat != MH_OK && stat != MH_ERROR_ALREADY_INITIALIZED)                                          \
    {                                                                                                   \
        GLUMITY_PRINT_COLOR(CON_RED, "Failed to initialize Minhook! (Status: %d)\n", pluginName, stat); \
        onFail;                                                                                         \
    }                                                                                                   \
    else

typedef MH_STATUS (*GlumityV2_TrackedCreateHook_t)(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal);
static GlumityV2_TrackedCreateHook_t g_TrackedHookBridgePtr = NULL;

inline void GlumityV2_TrackedCreateHook_OverBRidge()
{
    if (g_TrackedHookBridgePtr == NULL)
    {
        HMODULE hModule = GetModuleHandleA("TCCBridge-rdr2.dll");
        if (hModule != NULL)
        {
            g_TrackedHookBridgePtr = (GlumityV2_TrackedCreateHook_t)GetProcAddress(hModule, "GlumityV2_TrackedCreateHook");
        }
    }
}

#define GLUMITYV2_GAME_HOOK_CREATE(name, original, hook)                                                     \
    {                                                                                                        \
        GlumityV2_TrackedCreateHook_OverBRidge();                                                            \
        MH_STATUS stat = MH_ERROR_NOT_INITIALIZED;                                                           \
        if (g_TrackedHookBridgePtr != NULL)                                                                  \
        {                                                                                                    \
            stat = g_TrackedHookBridgePtr((LPVOID)(original), (LPVOID)(&hook), (LPVOID *)&(original));       \
        }                                                                                                    \
        else                                                                                                 \
        {                                                                                                    \
            stat = MH_CreateHook((LPVOID)(original), (LPVOID)(&hook), (LPVOID *)&(original));                \
        }                                                                                                    \
        if (stat == MH_OK)                                                                                   \
        {                                                                                                    \
            GLUMITY_PRINT_COLOR(CON_BLUE, "Created a hook: %s\n", GLUMITYLIB_PRINT_HEADER, name);            \
            GlumityV2_MakeHookAtomic((void **)&original, (void *)hook);                                      \
        }                                                                                                    \
        else if (stat == MH_ERROR_ALREADY_CREATED)                                                           \
        {                                                                                                    \
            GLUMITY_PRINT_COLOR(CON_YELLOW, "Hook already exists for: %s\n", GLUMITYLIB_PRINT_HEADER, name); \
        }                                                                                                    \
    }

#define GLUMITYV2_GAME_HOOK_ENABLE_ALL(pluginName)                              \
    {                                                                           \
        MH_STATUS stat = MH_EnableHook(MH_ALL_HOOKS);                           \
        if (stat == MH_OK)                                                      \
            GLUMITY_PRINT_COLOR(CON_GREEN, "Enabled all hooks!\n", pluginName); \
    }

#define GLUMITYV2_GAME_HOOK_ENABLE(name, original, pluginName)                                     \
    {                                                                                              \
        MH_STATUS stat = MH_EnableHook(original);                                                  \
        if (stat == MH_OK)                                                                         \
            GLUMITY_PRINT_COLOR(CON_GREEN, "Enabled a hook: %s\n", GLUMITYLIB_PRINT_HEADER, name); \
    }

#define GLUMITYV2_GAME_HOOK_TYPE(retType, name) typedef retType(*name)
#define GLUMITYV2_FUNC_TYPE(retType, name) typedef retType(*name)

#define GLUMITYV2_GAME_HOOK_CLEAN_ALL() \
    MH_DisableHook(MH_ALL_HOOKS);       \
    MH_RemoveHook(MH_ALL_HOOKS);

#endif
