#ifndef GLUMITYLIB_H
#define GLUMITYLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <MinHook.h>
#include <windef.h>

#define GLUMITYLIB_PRINT_HEADER "GlumityV2Lib"

#define GLUMITYV2_MODULE "GlumityToolSuite2"
#define GLUMITYV2_DLL "GlumityToolSuite2.dll"

#ifdef __cplusplus
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT __declspec(dllexport)
#endif
#ifdef __cplusplus
extern "C"
{
#endif

#include "GlumityPluginLoader.h"
#include "GlumityPlugin.h"
#include "GlumityFilesystem.h"
#include "GlumityHooking.h"

    BOOL Glumity_GetErrorMessage(DWORD dwErrorCode, LPTSTR pBuffer, DWORD cchBufferLength);

#define INIT_GLUMITYV2_EXPORT(mod, exportName, exportType) \
    (exportType) GetProcAddress(mod, exportName);

#ifdef __cplusplus
}
#endif

typedef struct IL2CPP_String__Fields
{
    int32_t m_stringLength;
    uint16_t m_firstChar;
} GlumityV2_il2cppStr_fields;

typedef struct IL2CPP_String
{
    void *klass;
    void *monitor;
    struct IL2CPP_String__Fields fields;
} GlumityV2_il2cppStr;

/// @brief Allocates memory, caller has to manage returned pointer
/// @param str a pointer to an initialized il2cpp string
/// @return pointer to a newly allocated ascii, null terminated c string, il2cpp strings are normally utf encoded, so it could be lossy
inline char *IL2CPP_String_ToCString(GlumityV2_il2cppStr *str)
{
    int len = str->fields.m_stringLength;
    uint16_t *chars = &str->fields.m_firstChar;

    // TODO: Check for memory leaks, I believe the game manages it automatically though
    char *result = (char *)malloc(len + 1);
    if (!result)
        return NULL;

    for (int i = 0; i < len; i++)
    {
        result[i] = (char)chars[i];
    }

    result[len] = '\0';
    return result;
}

/// @brief Allocates memory, caller has to manage returned pointer, can be ignored as well and let the game "manage" it, might cause leaks eventually
/// @param input Any null terminated c string
/// @return A pointer to a newly allocated il2cpp string
inline GlumityV2_il2cppStr *IL2CPP_String_Create(const char *input)
{
    if (!input)
        return NULL;

    size_t len = strlen(input);
    size_t size = sizeof(GlumityV2_il2cppStr) + (len * sizeof(uint16_t));

    GlumityV2_il2cppStr *str = (GlumityV2_il2cppStr *)malloc(size);
    if (!str)
        return NULL;

    str->klass = NULL;
    str->monitor = NULL;

    str->fields.m_stringLength = (int32_t)len;

    uint16_t *buffer = &str->fields.m_firstChar;

    for (size_t i = 0; i < len; i++)
    {
        buffer[i] = (uint16_t)input[i];
    }

    return str;
}

#endif