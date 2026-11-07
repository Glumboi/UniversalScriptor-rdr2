#ifndef PLUGIN_H
#define PLUGIN_H

#include <Windows.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "GlumityConsole.h"

#define DLL_PATH_MAX 260
#define GLUMITY_MAX_DEPS 16

// Handle dll export decorator for pure C compilation
#ifndef EXPORT
#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
#endif

typedef void (*GlumityPlugin_rdr2_EntryPoint)(HMODULE bridgeInstance);
typedef void (*GlumityPlugin_EntryPoint)(void);
typedef BOOL (*GlumityPlugin_ExitPoint)(void);
typedef void (*GlumityGeneric_Func)(void);
typedef void (*GlumityDependencyErrorCallback)(const char *failedDllName);

#define GLUMITYV2_VERIFY_DEPENDENCY(dllName)                                                                          \
    if (!GetModuleHandleA(dllName))                                                                                   \
        GLUMITY_PRINT_COLOR(CON_RED, "Missing dependency: %s, functionality may be impaired!\n", MY_PLUGIN, dllName); \
    else                                                                                                              \
        GLUMITY_PRINT_COLOR(CON_GREEN, "Found dependency: %s!\n", MY_PLUGIN, dllName);

#define GLUMITYV2_PLUGIN_ENTRY \
    EXPORT void GlumityMain(void)

#define GLUMITYV2_PLUGIN_EXIT \
    EXPORT BOOL GlumityExit(void) // return true/1 if finished cleaning up all hooks

// Recommended to use this to run most code,
// specially function hooking since you have to rely on the dumper being loaded
#define GLUMITYV2_PLUGIN_THREADRUN(function, pParam)                                         \
    {                                                                                        \
        freopen("CONOUT$", "w", stdout); /* enable plugin printing */                        \
        HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)function, pParam, 0, 0); \
        if (hThread)                                                                         \
            CloseHandle(hThread);                                                            \
    }

/// @brief Printf for glumity plugin specific code, uses malloc and free for a temp char*, caller doesn't need to manage
/// @param fmt
/// @param
void GlumityPlugin_printf(const char *fmt, const char *printHeaderInner, ...);

void GetPluginDirectory(HMODULE hModule, char *outBuffer, DWORD bufferSize);

/**
 * @brief Prints a message in a specific color and immediately restores the original color layout.
 * @param color Standard Windows console attribute color code (e.g., CON_RED)
 * @param fmt Format string (just like printf)
 * @param ... Variable arguments for formatting
 */
#define GLUMITY_PRINT_COLOR(color, fmt, header, ...)           \
    do                                                         \
    {                                                          \
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);     \
        CONSOLE_SCREEN_BUFFER_INFO csbi;                       \
        WORD originalAttributes = CON_RESET;                   \
        if (GetConsoleScreenBufferInfo(hConsole, &csbi))       \
        {                                                      \
            originalAttributes = csbi.wAttributes;             \
        }                                                      \
        SetConsoleTextAttribute(hConsole, color);              \
        GlumityPlugin_printf(fmt, header, ##__VA_ARGS__);      \
        SetConsoleTextAttribute(hConsole, originalAttributes); \
    } while (0)

typedef struct
{
    char dllPath[DLL_PATH_MAX];
    char *name;
    GlumityPlugin_EntryPoint entryPoint;
    GlumityPlugin_rdr2_EntryPoint entryPointRdr;
    GlumityPlugin_ExitPoint exitPoint;

    HMODULE hDll;

    // For the bridge only
    void *tccState;
    size_t compiledSz;
    void *programBase;
} GlumityPlugin;

#endif