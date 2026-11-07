/*
	C binding for ScriptHookRDR2 (see main.h).
	Add this file to your project. Call shv_init() from DllMain on DLL_PROCESS_ATTACH.

	NOTE: the decorated names below are what MSVC x64 generates for the SDK's C++
	declarations. They were derived by hand and are NOT verified against a real
	ScriptHookRDR2.dll. If shv_init() fails, dump the DLL's exports
	(dumpbin /exports ScriptHookRDR2.dll) and fix the string that is missing.
*/

#ifndef SHV_USE_IMPORT_LIB

#include "main.h"

#define SHV_DEFINE(ret, name, args) ret (*name) args = NULL

SHV_DEFINE(void, keyboardHandlerRegister, (KeyboardHandler));
SHV_DEFINE(void, keyboardHandlerUnregister, (KeyboardHandler));
SHV_DEFINE(void, scriptWait, (DWORD));
SHV_DEFINE(void, scriptRegister, (HMODULE, void(*)(void)));
SHV_DEFINE(void, scriptRegisterAdditionalThread, (HMODULE, void(*)(void)));
SHV_DEFINE(void, scriptUnregister, (HMODULE));
SHV_DEFINE(void, scriptUnregisterFn, (void(*)(void)));
SHV_DEFINE(void, nativeInit, (UINT64));
SHV_DEFINE(void, nativePush64, (UINT64));
SHV_DEFINE(PUINT64, nativeCall, (void));
SHV_DEFINE(UINT64*, getGlobalPtr, (int));
SHV_DEFINE(int, worldGetAllVehicles, (int*, int));
SHV_DEFINE(int, worldGetAllPeds, (int*, int));
SHV_DEFINE(int, worldGetAllObjects, (int*, int));
SHV_DEFINE(int, worldGetAllPickups, (int*, int));
SHV_DEFINE(BYTE*, getScriptHandleBaseAddress, (int));
SHV_DEFINE(eGameVersion, getGameVersion, (void));

#define SHV_BIND(fn, mangled) \
	do { *(FARPROC*)&fn = GetProcAddress(h, mangled); if (!fn) ok = FALSE; } while (0)

BOOL shv_init(void)
{
	BOOL ok = TRUE;
	HMODULE h = GetModuleHandleA("ScriptHookRDR2.dll");
	if (!h) return FALSE;

	SHV_BIND(keyboardHandlerRegister,       "?keyboardHandlerRegister@@YAXP6AXKGEHHHH@Z@Z");
	SHV_BIND(keyboardHandlerUnregister,     "?keyboardHandlerUnregister@@YAXP6AXKGEHHHH@Z@Z");
	SHV_BIND(scriptWait,                    "?scriptWait@@YAXK@Z");
	SHV_BIND(scriptRegister,                "?scriptRegister@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
	SHV_BIND(scriptRegisterAdditionalThread,"?scriptRegisterAdditionalThread@@YAXPEAUHINSTANCE__@@P6AXXZ@Z");
	SHV_BIND(scriptUnregister,              "?scriptUnregister@@YAXPEAUHINSTANCE__@@@Z");
	SHV_BIND(scriptUnregisterFn,            "?scriptUnregister@@YAXP6AXXZ@Z");
	SHV_BIND(nativeInit,                    "?nativeInit@@YAX_K@Z");
	SHV_BIND(nativePush64,                  "?nativePush64@@YAX_K@Z");
	SHV_BIND(nativeCall,                    "?nativeCall@@YAPEA_KXZ");
	SHV_BIND(getGlobalPtr,                  "?getGlobalPtr@@YAPEA_KH@Z");
	SHV_BIND(worldGetAllVehicles,           "?worldGetAllVehicles@@YAHPEAHH@Z");
	SHV_BIND(worldGetAllPeds,               "?worldGetAllPeds@@YAHPEAHH@Z");
	SHV_BIND(worldGetAllObjects,            "?worldGetAllObjects@@YAHPEAHH@Z");
	SHV_BIND(worldGetAllPickups,            "?worldGetAllPickups@@YAHPEAHH@Z");
	SHV_BIND(getScriptHandleBaseAddress,    "?getScriptHandleBaseAddress@@YAPEAEH@Z");
	SHV_BIND(getGameVersion,                "?getGameVersion@@YA?AW4eGameVersion@@XZ");

	return ok;
}

#endif /* SHV_USE_IMPORT_LIB */
