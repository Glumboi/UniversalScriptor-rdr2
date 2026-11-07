#ifndef GLUMITY_FILESYSTEM_H
#define GLUMITY_FILESYSTEM_H

#include <windows.h>
#include <stdio.h>

/// @brief Caller has to free
/// @param dir
/// @param outCount
/// @return allocated using malloc/realloc
char **Glumity_FileSystem_GetAllDllFilesFromDirectory(const char *dir, int *outCount);
char **Glumity_FileSystem_GetAllFilesWithExtensionFromDirectory(const char *dir, const char *extension, int *outCount);
const char *Glumity_GetFileNameFromPath(const char *path);

#endif