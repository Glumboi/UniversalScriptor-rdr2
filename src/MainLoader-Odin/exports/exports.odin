package exports

import c "core:c/libc"
import "core:fmt"
import "core:strconv"
import "core:strings"
import "core:sys/windows"

// Could be problematic if called by a plugin, never worked but lets export it anyway
@(export)
Glumity_printf :: proc(format: cstring, args: ..any) {
	c.printf("[GlumityV2]: ")
	str_builder := strings.builder_make()
	defer strings.builder_destroy(&str_builder)
	for arg in args {
		s := fmt.tprintf("%v", arg) // turn `any` into a string
		strings.write_string(&str_builder, s)
		strings.write_string(&str_builder, " ")
	}
	c.printf(format, strings.to_string(str_builder))
}