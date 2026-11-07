package helpers

import c "core:c/libc"
import "core:os"
import "core:strings"

string_multi_split_by_rune :: proc(s: string, sep: rune) -> [dynamic]string {
	parts := make_dynamic_array([dynamic]string) // empty slice, but with some capacity
	last := 0
	for i := 0; i < len(s); i += 1 {
		if rune(s[i]) == sep {
			append_elem(&parts, s[last:i])
			last = i + 1
		}
	}
	// add the final piece (after the last separator)
	if last < len(s) {
		append_elem(&parts, s[last:])
	}
	return parts
}
