package helpers

import c "core:c/libc"
import "core:sys/windows"

init_console :: proc() -> bool {
	res := windows.AllocConsole()
	if res {
		c.freopen("conout$", "w", c.stdout)
	}
	return bool(res)
}
