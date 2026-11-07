package helpers

import c "core:c/libc"
import "core:strings"

is_in_slice :: proc($T: typeid, arr: []$A, value: $V) -> bool {

	for x in arr {
		if x == value {
			return true
		}
	}
	return false
}


