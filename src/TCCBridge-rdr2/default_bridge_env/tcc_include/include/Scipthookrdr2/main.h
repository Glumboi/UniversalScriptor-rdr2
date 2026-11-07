/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019

	C port. See main.c for how the Script Hook functions are bound.
*/

#pragma once

#include <windows.h>

#define IMPORT __declspec(dllimport)

/*
	ScriptHookRDR2.dll exports C++-mangled names (e.g. ?scriptWait@@YAXK@Z), which a C
	compiler cannot reference through an import library. Three modes:

	1) Compiled by the TCC JIT (__TINYC__):
	   the functions are ordinary prototypes. The bridge (main.cpp) resolves them with
	   tcc_add_symbol() to its tcc_proxy_* functions. Nothing to do in scripts.

	2) Default (bridge DLL itself, main.c):
	   the functions are function pointers that shv_init() (main.c) fills in with
	   GetProcAddress. Call syntax is unchanged: scriptWait(0);
	   shv_init() returns FALSE if ScriptHookRDR2.dll isn't loaded or a function
	   could not be found.

	3) SHV_USE_IMPORT_LIB defined:
	   link with a C-callable build of the import library instead.
*/

#if defined(__TINYC__)

#define SHV_FN(ret, name, args) ret name args

#elif defined(SHV_USE_IMPORT_LIB)

#define SHV_FN(ret, name, args) IMPORT ret name args

#else

#define SHV_FN(ret, name, args) extern ret(*name) args

#ifdef __cplusplus
extern "C"
{
#endif
	BOOL shv_init(void);
#ifdef __cplusplus
}
#endif

#endif

/* keyboard */

/* DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow */
typedef void (*KeyboardHandler)(DWORD, WORD, BYTE, BOOL, BOOL, BOOL, BOOL);

/* Register keyboard handler
   must be called on dll attach */
SHV_FN(void, keyboardHandlerRegister, (KeyboardHandler handler));

/* Unregister keyboard handler
   must be called on dll detach */
SHV_FN(void, keyboardHandlerUnregister, (KeyboardHandler handler));

/* scripts */

SHV_FN(void, scriptWait, (DWORD time));
SHV_FN(void, scriptRegister, (HMODULE module, void (*LP_SCRIPT_MAIN)(void)));
SHV_FN(void, scriptRegisterAdditionalThread, (HMODULE module, void (*LP_SCRIPT_MAIN)(void)));
SHV_FN(void, scriptUnregister, (HMODULE module));

SHV_FN(void, nativeInit, (UINT64 hash));
SHV_FN(void, nativePush64, (UINT64 val));
SHV_FN(PUINT64, nativeCall, (void));

static void WAIT(DWORD time) { scriptWait(time); }
static void TERMINATE(void) { WAIT(MAXDWORD); }

/* Returns pointer to global variable
   make sure that you check game version before accessing globals because
   ids may differ between patches */
SHV_FN(UINT64 *, getGlobalPtr, (int globalId));
/*IMPORT UINT64 *getStaticPtr(const char* scriptName, int staticIndex);*/
/*IMPORT void switchLabel(const char* oldLabel, const char* newLabel);*/
/*IMPORT void* getCommandFromHash(uint64_t hash);*/

/* world */

/* Get entities from internal pools
   return value represents filled array elements count
   can be called only in the same thread as natives */
SHV_FN(int, worldGetAllVehicles, (int *arr, int arrSize));
SHV_FN(int, worldGetAllPeds, (int *arr, int arrSize));
SHV_FN(int, worldGetAllObjects, (int *arr, int arrSize));
SHV_FN(int, worldGetAllPickups, (int *arr, int arrSize));

/* misc */

/* Returns base object pointer using it's script handle
   make sure that you check game version before accessing object fields because
   offsets may differ between patches */
SHV_FN(BYTE *, getScriptHandleBaseAddress, (int handle));

typedef int eGameVersion;
enum
{
	VER_AUTO,
	VER_1_0_1207_60_RGS,
	VER_1_0_1207_69_RGS,

	VER_SIZE,
	VER_UNK = -1
};

SHV_FN(eGameVersion, getGameVersion, (void));