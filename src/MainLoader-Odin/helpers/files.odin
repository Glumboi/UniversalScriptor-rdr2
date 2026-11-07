package helpers

import "core:os"
import "core:path/filepath"
import "core:strings"

has_file_extension :: proc(file: ^os.File_Info, fileExtension: string) -> bool {

	if file.is_dir {return false}

	dot_idx := strings.last_index_byte(file.name, '.')
	if dot_idx == -1 {
		return false
	}

	extension := file.name[dot_idx:len(file.name)]
	return extension == fileExtension
}

remove_file_extension :: proc(file: string) -> string {
	dot_idx := strings.last_index(file, ".")

	if dot_idx == -1 {
		return file
	}

	return file[0:dot_idx]
}

get_file_name :: proc(file: string) -> string {

	if len(file) <= 0 {
		return ""
	}

	slash_idx := strings.last_index_any(file, "/\\")
	if slash_idx == -1 {
		return file
	}

	return file[slash_idx + 1:]
}
