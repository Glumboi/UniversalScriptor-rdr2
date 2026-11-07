#pragma once

#include <map>
#include <GlumityLib.h>
#include "include/libtcc.h"
extern "C"
{
#include "include/Scipthookrdr2/main.h"
}
#include <vector>
#include <string>
#include <set>

inline bool g_verboseBridge = false;
inline bool g_isReloading = false;

extern std::set<void *> g_trackedHookTargets;

inline MH_STATUS GlumityTrackedCreateHook(LPVOID pTarget, LPVOID pDetour, LPVOID *ppOriginal)
{
    if (pTarget)
    {
        g_trackedHookTargets.insert(pTarget);
    }
    return MH_CreateHook(pTarget, pDetour, ppOriginal);
}

inline std::map<std::string, GlumityGeneric_Func> hardApiFuncs = {
    {"GlumityPlugin_printf", reinterpret_cast<GlumityGeneric_Func>(&GlumityPlugin_printf)},
    {"Glumity_GetErrorMessage", reinterpret_cast<GlumityGeneric_Func>(&Glumity_GetErrorMessage)},
    {"MH_CreateHook", reinterpret_cast<GlumityGeneric_Func>(&GlumityTrackedCreateHook)},
    {"MH_EnableHook", reinterpret_cast<GlumityGeneric_Func>(&MH_EnableHook)},
    {"MH_Initialize", reinterpret_cast<GlumityGeneric_Func>(&MH_Initialize)},
    {"MH_DisableHook", reinterpret_cast<GlumityGeneric_Func>(&MH_DisableHook)},
    {"MH_RemoveHook", reinterpret_cast<GlumityGeneric_Func>(&MH_RemoveHook)},
};

inline const std::vector<std::string> G_HARD_LIBS = {"msvcrt", "kernel32", "user32"};
inline std::map<void *, GlumityPlugin> g_compiledScripts = {};