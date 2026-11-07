package dllLoader

import "../cppDumper"
import "../exports"
import "../helpers"
import c "core:c/libc"
import "core:os"
import "core:path"
import "core:path/filepath"
import "core:strings"
import "core:sys/windows"

dllLoader_generic_function :: distinct proc()

GLUMITY_ENTRY :: "GlumityMain"
GLUMITY_EXIT :: "GlumityExit"

PRIORITY_DLLS :: [?]string{
	"GlumityV2IL2CPPDumper.dll",
	"libtcc.dll",
}

dllLoader :: struct {
	loadedDlls:  map[string]windows.HMODULE,
	dllsToLoad:  map[string]bool,
	pluginsPath: string,
}

dllLoader_create :: proc() -> dllLoader {
	return {
		loadedDlls = make_map(map[string]windows.HMODULE),
		dllsToLoad = make_map(map[string]bool),
	}
}

dllLoader_getDllsToLoad :: proc(
	loader: ^dllLoader,
	locationToSearch: string,
	blockList: []string,
) {
	if loader == nil {return}

	if !os.exists(locationToSearch) || !os.is_dir(locationToSearch) {
		os.make_directory(locationToSearch)
		return
	}

	dirHandle, openErr := os.open(locationToSearch)
	if openErr != nil {
		exports.Glumity_printf(
			"Failed to open directory %s\n",
			strings.unsafe_string_to_cstring(locationToSearch),
		)
		return
	}
	defer os.close(dirHandle)

	allocator := os.heap_allocator

	files, readErr := os.read_dir(dirHandle, -1)
	if readErr != nil {
		exports.Glumity_printf(
			"Error reading directory (%s)!\n",
			strings.unsafe_string_to_cstring(locationToSearch),
		)
	}

	if len(files) == 0 {
		return
	}

	for &file in files {
		if helpers.has_file_extension(&file, ".dll") {
			if helpers.is_in_slice([]string, blockList, file.name) {
				exports.Glumity_printf(
					"Blocked dll: %s\n",
					strings.unsafe_string_to_cstring(file.name),
				)
				continue
			}
			forwardSlashed, _ := filepath.to_slash(file.fullpath)
			loader.dllsToLoad[forwardSlashed] = true
		}
	}
}

@(private)
dllLoader_LoadOldDll :: proc(loader: ^dllLoader, file: string, requestFile: string) {
	if len(file) <= 0 {return}

	fileName := helpers.get_file_name(file)
	exports.Glumity_printf(
		"%s is an old plugin, functionality could be broken!\n",
		strings.unsafe_string_to_cstring(file),
	)

	cppDumper.old_plugin_supply_request(requestFile)
}

dllLoader_LoadIL2CPPAPIBridge_TCC :: proc(loader: ^dllLoader) {
	tccPath := strings.concatenate({loader.pluginsPath, "/libtcc.dll"})

	if !os.exists(tccPath) {
		return
	}

	mod := windows.LoadLibraryA(strings.unsafe_string_to_cstring(tccPath))
	if mod == nil {
		lastErr := windows.GetLastError()
		exports.Glumity_printf(
			"Could not load libtcc [%s] error: %d\n",
			strings.unsafe_string_to_cstring(tccPath),
			lastErr,
		)
		return
	}

	loader.loadedDlls[tccPath] = mod
}

dllLoader_LoadIL2CPPDumper :: proc(loader: ^dllLoader) {
	dumperPath := strings.concatenate({loader.pluginsPath, "/GlumityV2IL2CPPDumper.dll"})

	if !os.exists(dumperPath) {
		return
	}

	mod := windows.LoadLibraryA(strings.unsafe_string_to_cstring(dumperPath))
	if mod == nil {
		lastErr := windows.GetLastError()
		exports.Glumity_printf(
			"Could not load GlumityV2IL2CPPDumper [%s] error: %d\n",
			strings.unsafe_string_to_cstring(dumperPath),
			lastErr,
		)
		return
	}

	procAdd := windows.GetProcAddress(mod, strings.unsafe_string_to_cstring(GLUMITY_ENTRY))
	if procAdd == nil {
		exports.Glumity_printf(
			"Could not find %s in the dumper plugin!\n",
			strings.unsafe_string_to_cstring(GLUMITY_ENTRY),
		)
		return
	}

	cppDumper.il2cpp_dumper_find_exports()
	loader.loadedDlls[dumperPath] = mod
}

dllLoader_loadDlls :: proc(loader: ^dllLoader) {
	if loader == nil {return}
	if len(loader.dllsToLoad) <= 0 {return}

	for priorityPath in PRIORITY_DLLS {
		fullPath := strings.concatenate({loader.pluginsPath, "/", priorityPath})
		defer delete(fullPath)
		
		forwardSlashed, _ := filepath.to_slash(fullPath)

		if !(forwardSlashed in loader.dllsToLoad) {continue}

		handle := windows.GetModuleHandleA(strings.unsafe_string_to_cstring(forwardSlashed))
		if handle != nil {
			delete_key(&loader.dllsToLoad, forwardSlashed)
			continue
		}

		exports.Glumity_printf("Trying to load priority: %s\n", strings.unsafe_string_to_cstring(forwardSlashed))

		mod := windows.LoadLibraryA(strings.unsafe_string_to_cstring(forwardSlashed))
		if mod == nil {
			lastErr := windows.GetLastError()
			exports.Glumity_printf(
				"Could not load priority library [%s] error: %d\n",
				strings.unsafe_string_to_cstring(forwardSlashed),
				lastErr,
			)
			delete_key(&loader.dllsToLoad, forwardSlashed)
			continue
		}

		loader.loadedDlls[forwardSlashed] = mod
		delete_key(&loader.dllsToLoad, forwardSlashed)
	}

	for k, _ in loader.dllsToLoad {
		handle := windows.GetModuleHandleA(strings.unsafe_string_to_cstring(k))
		if handle != nil {continue}

		exports.Glumity_printf("Trying to load: %s\n", strings.unsafe_string_to_cstring(k))

		mod := windows.LoadLibraryA(strings.unsafe_string_to_cstring(k))
		if mod == nil {
			lastErr := windows.GetLastError()
			exports.Glumity_printf(
				"Could not load library [%s] error: %d\n",
				strings.unsafe_string_to_cstring(k),
				lastErr,
			)
			continue
		}

		loader.loadedDlls[k] = mod
	}
}

dllLoader_call_exported_from_dlls :: proc(loader: ^dllLoader, exportToCall: string) {
	if loader == nil {return}

	for _, mod in loader.loadedDlls {
		if mod == nil {continue}
		procAdd := windows.GetProcAddress(mod, strings.unsafe_string_to_cstring(exportToCall))
		if procAdd == nil {continue}

		(dllLoader_generic_function(procAdd))()
	}
}

dllLoader_dispose :: proc(loader: ^dllLoader) {
	if loader == nil {return}
	dllLoader_call_exported_from_dlls(loader, GLUMITY_EXIT)

	for dll, mod in loader.loadedDlls {
		windows.FreeLibrary(mod)
	}

	delete_map(loader.loadedDlls)
	delete_map(loader.dllsToLoad)
}