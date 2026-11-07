/*
	THIS FILE IS A PART OF RDR 2 SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2019

	C port. See main.c for how the Script Hook functions are bound.
*/

#pragma once

#include <windows.h>

#define IMPORT __declspec(dllimport)
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

typedef void (*KeyboardHandler)(DWORD, WORD, BYTE, BOOL, BOOL, BOOL, BOOL);

SHV_FN(void, keyboardHandlerRegister, (KeyboardHandler handler));

SHV_FN(void, keyboardHandlerUnregister, (KeyboardHandler handler));

SHV_FN(void, scriptWait, (DWORD time));
SHV_FN(void, scriptRegister, (HMODULE module, void (*LP_SCRIPT_MAIN)(void)));
SHV_FN(void, scriptRegisterAdditionalThread, (HMODULE module, void (*LP_SCRIPT_MAIN)(void)));
SHV_FN(void, scriptUnregister, (HMODULE module));

SHV_FN(void, nativeInit, (UINT64 hash));
SHV_FN(void, nativePush64, (UINT64 val));
SHV_FN(PUINT64, nativeCall, (void));

static void WAIT(DWORD time) { scriptWait(time); }
static void TERMINATE(void) { WAIT(MAXDWORD); }

SHV_FN(UINT64 *, getGlobalPtr, (int globalId));

SHV_FN(int, worldGetAllVehicles, (int *arr, int arrSize));
SHV_FN(int, worldGetAllPeds, (int *arr, int arrSize));
SHV_FN(int, worldGetAllObjects, (int *arr, int arrSize));
SHV_FN(int, worldGetAllPickups, (int *arr, int arrSize));

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