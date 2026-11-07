package cppDumper

import "core:encoding/json"
import "core:fmt"
import "core:mem"
import "core:os"
import "core:strconv"
import "core:strconv/decimal"
import "core:strings"
import "core:sys/windows"

import c "core:c/libc"

// Define the structures matching your JSON
offsetRequest :: struct {
	class:    string,
	function: string,
}

/*
// example of an old mod
{
  "caller": "PCBS2-Cheats",
  "offsetRequests": [
    {
      "searchName": "CareerStatus_State_OnDeserialization",
      "value": "0xCB77C0",
      "signature": "void CareerStatus_State__OnDeserialization (CareerStatus_State_o* __this, Il2CppObject* sender, const MethodInfo* method);"
    }
  ]
}
*/

IL2CPPDUMPER_MODULE_NAME :: "GlumityV2IL2CPPDumper.dll"

@(private)
GlumityV2Dumper_WaitForDumper_t :: distinct proc()
GlumityV2Dumper_WaitForDumper: GlumityV2Dumper_WaitForDumper_t

@(private)
GlumityV2Dumper_GetFunctionPointer_t :: distinct proc(class: cstring, function: cstring) -> rawptr
GlumityV2Dumper_GetFunctionPointer: GlumityV2Dumper_GetFunctionPointer_t

il2cpp_dumper_find_exports :: proc() -> bool {
	dumperHmod := windows.GetModuleHandleA(IL2CPPDUMPER_MODULE_NAME)
	if dumperHmod == nil {
		return false
	}
	GlumityV2Dumper_WaitForDumper = GlumityV2Dumper_WaitForDumper_t(
		windows.GetProcAddress(dumperHmod, "GlumityV2Dumper_WaitForDumper"),
	)
	GlumityV2Dumper_GetFunctionPointer = GlumityV2Dumper_GetFunctionPointer_t(
		windows.GetProcAddress(dumperHmod, "GlumityV2Dumper_GetFunctionPointer"),
	)

	return true
}

il2cpp_dumper_get_function_ptr :: proc(class: cstring, function: cstring) -> rawptr {

	if GlumityV2Dumper_WaitForDumper == nil || GlumityV2Dumper_GetFunctionPointer == nil {
		return nil
	}

	GlumityV2Dumper_WaitForDumper()

	return GlumityV2Dumper_GetFunctionPointer(class, function)
}

// TODO: Fix and implement, meant to add support for my older v1 toolsuite that used a different way of getting function offsets at "runtime"
old_plugin_supply_request :: proc(requestFile: string) {

	if !os.exists(requestFile) {
		return
	}

	data, success := os.read_entire_file_from_filename(requestFile)
	if !success {
		return
	}
	defer delete(data)

	jsonData, err := json.parse(data)
	if err != nil {
		return
	}
	defer json.destroy_value(jsonData)

	root := jsonData.(json.Object)

	offsetRequests := root["offsetRequests"].(json.Array)
	for request in offsetRequests {
		reqObj := request.(json.Object)
		searchName := reqObj["searchName"].(json.String)

		// Find last '_'
		lastIdx := strings.last_index(searchName, "_")
		if lastIdx < 0 {
			continue
		}

		// Split into class part and function part
		classPart := searchName[0:lastIdx] // up to last '_'
		funcPart := searchName[lastIdx + 1:] // after last '_'

		// Merge classPart into camel-case (remove underscores, capitalize each word)
		words := strings.split(classPart, "_")
		camelClass: string
		for w in words {
			if len(w) == 0 {continue}
			camelClass = strings.concatenate({camelClass, strings.to_upper(w[0:1]), w[1:]})
		}

		req: offsetRequest = {
			class    = strings.clone(camelClass),
			function = strings.clone(funcPart),
		}

		ptr := il2cpp_dumper_get_function_ptr(
			strings.unsafe_string_to_cstring(camelClass),
			strings.unsafe_string_to_cstring(funcPart),
		)

		c.printf(
			"Class: %s | Function: %s\nPtr:%p\n",
			strings.unsafe_string_to_cstring(camelClass),
			strings.unsafe_string_to_cstring(funcPart),
			ptr,
		)
	}
}
