#include <GlumityLib.h>
#include <string.h>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <memoryapi.h>
#include <atomic>

#include "globals.h"
#include "defines.h"

struct DeadMemoryBlock
{
    void *address;
    void *tccState;
    char *name;
};
std::vector<DeadMemoryBlock> g_zombieBlocks;

std::map<std::string, GlumityPlugin> g_activePlugins;
std::set<void *> g_trackedHookTargets;

std::atomic<bool> g_keepKeyboardUpdateLoopAlive{true};

VOID TCC_Error_Handler(void *opaque, const char *msg)
{
    GLUMITY_PRINT_COLOR(CON_RED, "%s\n", TCC_HEADER, msg);
}

HMODULE GetCurrentModule()
{
    HMODULE hModule = NULL;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      (LPCTSTR)GetCurrentModule, &hModule);
    return hModule;
}

// The real functions are pointers filled in by shv_init() (Scipthookrdr2/main.c)
// see plain functions (Scipthookrdr2/main.h, __TINYC__ branch) that are resolved to these
extern "C"
{
    void tcc_proxy_keyboardHandlerRegister(KeyboardHandler h)
    {
        if (keyboardHandlerRegister)
            keyboardHandlerRegister(h);
    }
    void tcc_proxy_keyboardHandlerUnregister(KeyboardHandler h)
    {
        if (keyboardHandlerUnregister)
            keyboardHandlerUnregister(h);
    }
    void tcc_proxy_scriptWait(DWORD t)
    {
        if (scriptWait)
            scriptWait(t);
    }
    void tcc_proxy_scriptRegister(HMODULE m, void (*f)(void))
    {
        GLUMITY_PRINT_COLOR(CON_CYAN, "[DEBUG] scriptRegister called: module=%p fn=%p bound=%d\n", TCC_HEADER, (void *)m, (void *)f, scriptRegister != nullptr);
        if (scriptRegister)
            scriptRegister(m, f);
        GLUMITY_PRINT_COLOR(CON_CYAN, "[DEBUG] scriptRegister returned\n", TCC_HEADER);
    }
    void tcc_proxy_scriptRegisterAdditionalThread(HMODULE m, void (*f)(void))
    {
        if (scriptRegisterAdditionalThread)
            scriptRegisterAdditionalThread(m, f);
    }
    void tcc_proxy_scriptUnregister(HMODULE m)
    {
        if (scriptUnregister)
            scriptUnregister(m);
    }
    void tcc_proxy_nativeInit(UINT64 h)
    {
        if (nativeInit)
            nativeInit(h);
    }
    void tcc_proxy_nativePush64(UINT64 v)
    {
        if (nativePush64)
            nativePush64(v);
    }
    PUINT64 tcc_proxy_nativeCall(void)
    {
        return nativeCall ? nativeCall() : nullptr;
    }
    UINT64 *tcc_proxy_getGlobalPtr(int id)
    {
        return getGlobalPtr ? getGlobalPtr(id) : nullptr;
    }
    int tcc_proxy_worldGetAllVehicles(int *a, int n)
    {
        return worldGetAllVehicles ? worldGetAllVehicles(a, n) : 0;
    }
    int tcc_proxy_worldGetAllPeds(int *a, int n)
    {
        return worldGetAllPeds ? worldGetAllPeds(a, n) : 0;
    }
    int tcc_proxy_worldGetAllObjects(int *a, int n)
    {
        return worldGetAllObjects ? worldGetAllObjects(a, n) : 0;
    }
    int tcc_proxy_worldGetAllPickups(int *a, int n)
    {
        return worldGetAllPickups ? worldGetAllPickups(a, n) : 0;
    }
    BYTE *tcc_proxy_getScriptHandleBaseAddress(int h)
    {
        return getScriptHandleBaseAddress ? getScriptHandleBaseAddress(h) : nullptr;
    }
    int tcc_proxy_getGameVersion(void)
    {
        return getGameVersion ? (int)getGameVersion() : -1;
    }
}

struct ShvExport
{
    const char *name;
    const void *addr;
};

#define SHV_EXPORT(n) {#n, (const void *)&tcc_proxy_##n}
static const ShvExport kShvExports[] = {
    SHV_EXPORT(keyboardHandlerRegister),
    SHV_EXPORT(keyboardHandlerUnregister),
    SHV_EXPORT(scriptWait),
    SHV_EXPORT(scriptRegister),
    SHV_EXPORT(scriptRegisterAdditionalThread),
    SHV_EXPORT(scriptUnregister),
    SHV_EXPORT(nativeInit),
    SHV_EXPORT(nativePush64),
    SHV_EXPORT(nativeCall),
    SHV_EXPORT(getGlobalPtr),
    SHV_EXPORT(worldGetAllVehicles),
    SHV_EXPORT(worldGetAllPeds),
    SHV_EXPORT(worldGetAllObjects),
    SHV_EXPORT(worldGetAllPickups),
    SHV_EXPORT(getScriptHandleBaseAddress),
    SHV_EXPORT(getGameVersion),
};

void LoadAndRunSingleScript(const std::filesystem::path &scriptPath, const char *libDir, const char *includeDir)
{
    GLUMITY_PRINT_COLOR(CON_BLUE, "=== [DEBUG] Starting JIT Compilation for %s ===\n", TCC_HEADER, scriptPath.filename().string().c_str());

    TCCState *tccState = tcc_new();
    if (!tccState)
    {
        GLUMITY_PRINT_COLOR(CON_RED, "[DEBUG] Critical Error: Failed to create a new TCC state!\n", TCC_HEADER);
        return;
    }

    std::string scriptPathString = scriptPath.string();
    std::string fullOptions = "-g -L" + std::string(libDir) + "\\lib -I" + std::string(includeDir) + "\\include ";

    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] TCC Options: %s\n", TCC_HEADER, fullOptions.c_str());
    tcc_set_options(tccState, fullOptions.c_str());

    tcc_set_output_type(tccState, TCC_OUTPUT_MEMORY);
    tcc_set_error_func(tccState, 0x0, TCC_Error_Handler);

    // Register ScriptHook API + bridge API before parsing/relocating
    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Registering ScriptHook proxies via tcc_add_symbol...\n", TCC_HEADER);
    for (const auto &e : kShvExports)
    {
        tcc_add_symbol(tccState, e.name, e.addr);
    }

    for (auto &apiFunc : hardApiFuncs)
    {
        tcc_add_symbol(tccState, apiFunc.first.c_str(), (const void *)apiFunc.second);
    }

    for (auto &lib : G_HARD_LIBS)
    {
        GLUMITY_PRINT_COLOR(CON_MAGENTA, "[DEBUG] Adding library to context: %s\n", TCC_HEADER, lib.c_str());
        tcc_add_library(tccState, lib.c_str());
    }

    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Invoking tcc_add_file for source path...\n", TCC_HEADER);
    if (tcc_add_file(tccState, scriptPath.string().c_str()) < 0)
    {
        GLUMITY_PRINT_COLOR(CON_RED, "[DEBUG] Error: tcc_add_file failed during parsing pass.\n", TCC_HEADER);
        tcc_delete(tccState);
        return;
    }
    GLUMITY_PRINT_COLOR(CON_GREEN, "[DEBUG] tcc_add_file parsing phase completed successfully.\n", TCC_HEADER);

    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Executing tcc_relocate to finalize build matrix...\n", TCC_HEADER);
    int relocation_result = tcc_relocate(tccState, TCC_RELOCATE_AUTO);

    if (relocation_result < 0)
    {
        GLUMITY_PRINT_COLOR(CON_RED, "[DEBUG] Error: tcc_relocate aborted with status code %d\n", TCC_HEADER, relocation_result);
        tcc_delete(tccState);
        return;
    }
    GLUMITY_PRINT_COLOR(CON_GREEN, "[DEBUG] Relocation successful! Target size bound: %d bytes\n", TCC_HEADER, relocation_result);

    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Pulling entrypoints out of memory via tcc_get_symbol...\n", TCC_HEADER);
    GlumityPlugin plugin = {};
    plugin.entryPoint = (GlumityPlugin_EntryPoint)tcc_get_symbol(tccState, "GlumityMain");
    plugin.entryPointRdr = (GlumityPlugin_rdr2_EntryPoint)tcc_get_symbol(tccState, "RdrScriptMain");
    plugin.exitPoint = (GlumityPlugin_ExitPoint)tcc_get_symbol(tccState, "GlumityExit");

    GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Symbols: GlumityMain=%p RdrScriptMain=%p GlumityExit=%p\n", TCC_HEADER,
                        (void *)plugin.entryPoint, (void *)plugin.entryPointRdr, (void *)plugin.exitPoint);
    if (!plugin.entryPointRdr)
    {
        GLUMITY_PRINT_COLOR(CON_RED, "[DEBUG] RdrScriptMain not found in script - nothing will run!\n", TCC_HEADER);
    }

    if (scriptPathString.length() >= sizeof(plugin.dllPath))
    {
        tcc_delete(tccState);
        return;
    }

    strcpy_s(plugin.dllPath, sizeof(plugin.dllPath), scriptPathString.c_str());
    plugin.tccState = tccState;
    plugin.hDll = NULL;
    plugin.name = _strdup(scriptPath.filename().string().c_str());
    plugin.compiledSz = relocation_result;
    plugin.programBase = NULL;

    if (plugin.entryPointRdr)
    {
        g_activePlugins[scriptPathString] = plugin;
        HMODULE bridgeMod = GetCurrentModule();
        GLUMITY_PRINT_COLOR(CON_YELLOW, "[DEBUG] Calling RdrScriptMain(bridgeMod=%p)...\n", TCC_HEADER, (void *)bridgeMod);
        if (bridgeMod != nullptr)
        {
            plugin.entryPointRdr(bridgeMod);
            GLUMITY_PRINT_COLOR(CON_GREEN, "[DEBUG] RdrScriptMain returned\n", TCC_HEADER);
        }
        else
        {
            GLUMITY_PRINT_COLOR(CON_RED, "[DEBUG] GetCurrentModule() returned NULL!\n", TCC_HEADER);
        }
    }
}

VOID ProcessJITScriptsFolder(const char *targetProgramFolder, const char *libDir, const char *includeDir)
{
    std::filesystem::path scriptDir = std::filesystem::path(targetProgramFolder) / "jit_scripts";
    if (!std::filesystem::exists(scriptDir) || !std::filesystem::is_directory(scriptDir))
    {
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(scriptDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".c")
        {
            LoadAndRunSingleScript(entry.path(), libDir, includeDir);
        }
    }
}

VOID HotReloadJITScripts()
{
    HMODULE hCurrentDll = GetModuleHandleA("TCCBridge-rdr2.dll");

    char pathBuffer[MAX_PATH] = {0};
    GetPluginDirectory(hCurrentDll, pathBuffer, sizeof(pathBuffer));

    size_t len = strlen(pathBuffer);
    if (len > 0 && pathBuffer[len - 1] != '\\')
    {
        strcat_s(pathBuffer, sizeof(pathBuffer), "\\");
    }

    strcat_s(pathBuffer, sizeof(pathBuffer), "default_bridge_env\\");

    char libs[MAX_PATH];
    strcpy_s(libs, sizeof(libs), pathBuffer);
    char incs[MAX_PATH];
    strcpy_s(incs, sizeof(incs), pathBuffer);

    strcat_s(libs, sizeof(libs), "tcc_libs");
    strcat_s(incs, sizeof(incs), "tcc_include");

    g_isReloading = true;

    for (auto &[path, plugin] : g_activePlugins)
    {
        if (plugin.exitPoint)
            plugin.exitPoint();
    }

    for (void *targetAddress : g_trackedHookTargets)
    {
        if (targetAddress)
        {
            MH_DisableHook(targetAddress);
            MH_RemoveHook(targetAddress);
        }
    }
    g_trackedHookTargets.clear();

    Sleep(60);

    std::vector<std::string> pathsToReload;
    for (auto &[path, plugin] : g_activePlugins)
    {
        pathsToReload.push_back(path);
        g_zombieBlocks.push_back({plugin.programBase, plugin.tccState, plugin.name});
    }

    g_activePlugins.clear();

    for (const std::string &scriptPath : pathsToReload)
    {
        GLUMITY_PRINT_COLOR(CON_YELLOW, "Applying script updates: %s\n", MY_PLUGIN, scriptPath.c_str());
        LoadAndRunSingleScript(std::filesystem::path(scriptPath), libs, incs);
    }

    g_isReloading = false;
}

// CONSOLE WATCHER: the console may be gone, so this logs to a FILE
// (TCCBridge_console_watch.log in the game folder)
// state 0 = GetConsoleWindow()==NULL  -> the console was freed/detached (FreeConsole)
// state 1 = handle no longer a window -> destroyed
// state 2 = window exists but hidden
// state 3 = window minimized
// state 4 = window visible (if you still can't see it, it's behind the game window)
static void ConsoleWatchTick()
{
    static DWORD lastCheck = 0;
    static int lastState = -2;

    DWORD now = GetTickCount();
    if (now - lastCheck < 500)
        return;
    lastCheck = now;

    HWND w = GetConsoleWindow();
    int state = !w ? 0 : (!IsWindow(w) ? 1 : (!IsWindowVisible(w) ? 2 : (IsIconic(w) ? 3 : 4)));
    if (state == 2)
        ShowWindow(w, SW_SHOWNA); // something hid it: force it visible again
    if (state == lastState)
        return;
    lastState = state;

    FILE *f = nullptr;
    if (fopen_s(&f, "TCCBridge_console_watch.log", "a") == 0 && f)
    {
        fprintf(f, "[%lu ms] console state=%d hwnd=%p foregroundIsConsole=%d\n",
                (unsigned long)now, state, (void *)w, (int)(GetForegroundWindow() == w && w != nullptr));
        fclose(f);
    }
}

VOID KeyboardUpdateLoop()
{
    while (g_keepKeyboardUpdateLoopAlive)
    {
        ConsoleWatchTick();
        if ((GetAsyncKeyState(VK_END) & 0x8000) != 0)
        {
            GLUMITY_PRINT_COLOR(CON_CYAN, "End pressed, reloading jit_scripts...\n", MY_PLUGIN);
            HotReloadJITScripts();

            while ((GetAsyncKeyState(VK_END) & 0x8000) != 0)
            {
                Sleep(10);
            }
        }
        Sleep(10);
    }
}

// logs exceptions with address +module.
static PVOID g_vehHandle = nullptr;

static LONG CALLBACK BridgeVEH(PEXCEPTION_POINTERS ep)
{
    DWORD code = ep->ExceptionRecord->ExceptionCode;
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_PRIV_INSTRUCTION:
    case EXCEPTION_STACK_OVERFLOW:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case 0xC0000409: // fast-fail
        break;
    default:
        return EXCEPTION_CONTINUE_SEARCH;
    }

    void *addr = ep->ExceptionRecord->ExceptionAddress;

    static void *lastAddr = nullptr;
    static int repeatCount = 0;
    if (addr == lastAddr)
    {
        if (++repeatCount > 2)
            return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        lastAddr = addr;
        repeatCount = 0;
    }

    HMODULE mod = NULL;
    char modName[MAX_PATH] = "<unknown / JIT code>";
    size_t offset = 0;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)addr, &mod) && mod)
    {
        GetModuleFileNameA(mod, modName, sizeof(modName));
        offset = (size_t)((char *)addr - (char *)mod);
    }

    GLUMITY_PRINT_COLOR(CON_RED, "[EXCEPTION] (first-chance, may be handled) code=0x%08lX at %p  module=%s  +0x%llX  thread=%lu\n", TCC_HEADER,
                        code, addr, modName, (unsigned long long)offset, GetCurrentThreadId());
    GLUMITY_PRINT_COLOR(CON_RED, "[CRASH] Rsp=%p Rip=%p Rcx=%p Rdx=%p\n", TCC_HEADER,
                        (void *)ep->ContextRecord->Rsp, (void *)ep->ContextRecord->Rip,
                        (void *)ep->ContextRecord->Rcx, (void *)ep->ContextRecord->Rdx);
    if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2)
    {
        GLUMITY_PRINT_COLOR(CON_RED, "[CRASH] access violation: %s address %p\n", TCC_HEADER,
                            ep->ExceptionRecord->ExceptionInformation[0] == 0 ? "reading" : (ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "writing" : "executing"),
                            (void *)ep->ExceptionRecord->ExceptionInformation[1]);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

VOID Setup()
{
    if (!g_vehHandle)
        g_vehHandle = AddVectoredExceptionHandler(1, BridgeVEH);

    int retry_count = 0;
    while (!shv_init())
    {
        GLUMITY_PRINT_COLOR(CON_RED, "ScriptHookRDR2.dll could not be loaded or bound!\n", TCC_HEADER);

        if (retry_count++ > 50)
        {
            GLUMITY_PRINT_COLOR(CON_RED, "Fatal: ScriptHookRDR2.dll binding timed out. Aborting bridge initialization.\n", TCC_HEADER);
            return;
        }

        Sleep(60);
        GLUMITY_PRINT_COLOR(CON_GREEN, "Retrying to bind...\n", TCC_HEADER);
    }

    GLUMITY_PRINT_COLOR(CON_GREEN, "ScriptHookRDR2.dll successfully bound!\n", TCC_HEADER);

    HMODULE hCurrentDll = GetModuleHandleA("TCCBridge-rdr2.dll");
    char pathBuffer[MAX_PATH] = {0};
    GetPluginDirectory(hCurrentDll, pathBuffer, sizeof(pathBuffer));

    size_t len = strlen(pathBuffer);
    if (len > 0 && pathBuffer[len - 1] != '\\')
    {
        strcat_s(pathBuffer, sizeof(pathBuffer), "\\");
    }

    strcat_s(pathBuffer, sizeof(pathBuffer), "default_bridge_env\\");

    char libs[MAX_PATH];
    strcpy_s(libs, sizeof(libs), pathBuffer);
    char incs[MAX_PATH];
    strcpy_s(incs, sizeof(incs), pathBuffer);

    strcat_s(libs, sizeof(libs), "tcc_libs");
    strcat_s(incs, sizeof(incs), "tcc_include");

    GetPluginDirectory(hCurrentDll, pathBuffer, sizeof(pathBuffer));
    ProcessJITScriptsFolder(pathBuffer, libs, incs);
}

GLUMITYV2_PLUGIN_ENTRY
{
    GLUMITYV2_PLUGIN_THREADRUN(Setup, 0);
    GLUMITYV2_PLUGIN_THREADRUN(KeyboardUpdateLoop, NULL);
}

GLUMITYV2_PLUGIN_EXIT
{
    GLUMITYV2_GAME_HOOK_CLEAN_ALL();

    g_keepKeyboardUpdateLoopAlive = false;
    Sleep(50);

    if (g_vehHandle)
    {
        RemoveVectoredExceptionHandler(g_vehHandle);
        g_vehHandle = nullptr;
    }

    for (auto &zombie : g_zombieBlocks)
    {
        if (zombie.tccState)
            tcc_delete((TCCState *)zombie.tccState);
        if (zombie.address)
            VirtualFree(zombie.address, 0, MEM_RELEASE);
        if (zombie.name)
            free(zombie.name);
    }
    return 1;
}