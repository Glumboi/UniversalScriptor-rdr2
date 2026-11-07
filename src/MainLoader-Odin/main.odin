package MainLoader_Odin

import "base:runtime"
import c "core:c/libc"
import "core:strings"
import "core:sys/windows"
import "core:thread"
import "cppDumper"
import "dllLoader"
import config "dllLoaderConfig"
import "exports"
import "helpers"

foreign import kernel32 "system:Kernel32.lib"

@(default_calling_convention = "system")
foreign kernel32 {
	GetConsoleWindow :: proc() -> windows.HWND ---
}

console_watchdog :: proc "std" (param: rawptr) -> u32 {
	context = runtime.default_context()
	for {
		if GetConsoleWindow() == nil {
			if windows.AllocConsole() {
				c.freopen("conout$", "w", c.stdout)
				exports.Glumity_printf("Console re-created by watchdog\n")
			}
		}
		windows.Sleep(500)
	}
	return 0
}

g_pluginLoader: dllLoader.dllLoader
g_cfg: config.loaderConfig

setup :: proc() {
	g_cfg = config.loaderConfig_init("GlumityToolSuite2.ini")

	if g_cfg.useConsole && helpers.init_console() {
		exports.Glumity_printf("Initialized console...\n")
		windows.CreateThread(nil, 0, console_watchdog, nil, 0, nil)
	}

	g_pluginLoader = dllLoader.dllLoader_create()
	g_pluginLoader.pluginsPath = g_cfg.pluginPath
	dllLoader.dllLoader_getDllsToLoad(&g_pluginLoader, g_cfg.pluginPath, g_cfg.blockList)

	if g_cfg.loadDumperFirst {
		dllLoader.dllLoader_LoadIL2CPPDumper(&g_pluginLoader)
	}

	dllLoader.dllLoader_LoadIL2CPPAPIBridge_TCC(&g_pluginLoader);
	dllLoader.dllLoader_loadDlls(&g_pluginLoader)

	if len(g_pluginLoader.loadedDlls) <= 0 {
		exports.Glumity_printf(
			"No plugins found [%s\b]!\n",
			strings.unsafe_string_to_cstring(g_cfg.pluginPath),
		)
	} else {
		exports.Glumity_printf("Loaded dlls: \n")
		for dll, mod in g_pluginLoader.loadedDlls {
			c.printf("%p\n", mod)
		}
		dllLoader.dllLoader_call_exported_from_dlls(&g_pluginLoader, dllLoader.GLUMITY_ENTRY)
	}
}

cleanup :: proc() {
	defer config.loaderConfig_unload(&g_cfg)
	defer dllLoader.dllLoader_dispose(&g_pluginLoader)
}

setup_thread_adapter :: proc "std" (param: rawptr) -> u32 {
	context = runtime.default_context()
	setup() // run this function
	return 0 // thread exit code
}

main :: proc() {
	hThread := windows.CreateThread(
		nil, // lpThreadAttributes
		0, // dwStackSize
		setup_thread_adapter, // lpStartAddress
		nil, // lpParameter
		0, // dwCreationFlags
		nil, // lpThreadId
	)
	windows.CloseHandle(hThread)
}
