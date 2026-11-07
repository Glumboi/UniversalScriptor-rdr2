#include "GlumityFilesystem.h"
#include "GlumityLib.h"

char **Glumity_FileSystem_GetAllDllFilesFromDirectory(const char *dir, int *outCount)
{
    if (!dir)
        return NULL;

    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*.dll", dir);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    int capacity = 10;
    int count = 0;
    char **dllFiles = (char **)malloc(capacity * sizeof(char *));
    if (!dllFiles)
    {
        FindClose(hFind);
        return NULL;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            char fullPath[MAX_PATH];
            snprintf(fullPath, MAX_PATH, "%s\\%s", dir, findData.cFileName);

            if (count >= capacity)
            {
                capacity *= 2;
                char **temp = (char **)realloc(dllFiles, capacity * sizeof(char *));
                if (!temp)
                {
                    // Cleanup already allocated strings
                    for (int i = 0; i < count; ++i)
                        free(dllFiles[i]);
                    free(dllFiles);
                    FindClose(hFind);
                    return NULL;
                }
                dllFiles = temp;
            }

            dllFiles[count] = _strdup(fullPath); // or strdup depending on compiler
            if (!dllFiles[count])
            {
                for (int i = 0; i < count; ++i)
                    free(dllFiles[i]);
                free(dllFiles);
                FindClose(hFind);
                return NULL;
            }

            count++;
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    DWORD err = GetLastError();
    if (err != ERROR_NO_MORE_FILES)
    {
        GlumityPlugin_printf("FindNextFile failed (%lu)\n", GLUMITYLIB_PRINT_HEADER, err);
    }

    FindClose(hFind);

    if (count == 0)
    {
        free(dllFiles);
        return NULL;
    }

    if (outCount)
        *outCount = count;

    return dllFiles;
}

char **Glumity_FileSystem_GetAllFilesWithExtensionFromDirectory(const char *dir, const char *extension, int *outCount)
{
    if (!dir)
        return NULL;

    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*%s", dir, extension);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPath, &findData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return NULL;
    }

    int capacity = 10;
    int count = 0;
    char **files = (char **)malloc(capacity * sizeof(char *));
    if (!files)
    {
        FindClose(hFind);
        return NULL;
    }

    do
    {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            char fullPath[MAX_PATH];
            snprintf(fullPath, MAX_PATH, "%s\\%s", dir, findData.cFileName);

            if (count >= capacity)
            {
                capacity *= 2;
                char **temp = (char **)realloc(files, capacity * sizeof(char *));
                if (!temp)
                {
                    // Cleanup already allocated strings
                    for (int i = 0; i < count; ++i)
                        free(files[i]);
                    free(files);
                    FindClose(hFind);
                    return NULL;
                }
                files = temp;
            }

            files[count] = _strdup(fullPath); // or strdup depending on compiler
            if (!files[count])
            {
                for (int i = 0; i < count; ++i)
                    free(files[i]);
                free(files);
                FindClose(hFind);
                return NULL;
            }

            count++;
        }
    } while (FindNextFileA(hFind, &findData) != 0);

    DWORD err = GetLastError();
    if (err != ERROR_NO_MORE_FILES)
    {
        GlumityPlugin_printf("FindNextFile failed (%lu)\n", GLUMITYLIB_PRINT_HEADER, err);
    }

    FindClose(hFind);

    if (count == 0)
    {
        free(files);
        return NULL;
    }

    if (outCount)
        *outCount = count;

    return files;
}

const char *Glumity_GetFileNameFromPath(const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *backslash = strrchr(path, '\\');
    const char *filename = path;

    if (slash && backslash)
        filename = (slash > backslash) ? slash + 1 : backslash + 1;
    else if (slash)
        filename = slash + 1;
    else if (backslash)
        filename = backslash + 1;

    return filename;
}
