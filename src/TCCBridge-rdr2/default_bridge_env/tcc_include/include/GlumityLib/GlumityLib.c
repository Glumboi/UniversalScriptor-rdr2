#include "GlumityLib.h"

#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include <MinHook.h>

BOOL Glumity_GetErrorMessage(DWORD dwErrorCode, LPTSTR pBuffer, DWORD cchBufferLength)
{
    if (cchBufferLength == 0)
    {
        return FALSE;
    }

    DWORD cchMsg = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, /* (not used with FORMAT_MESSAGE_FROM_SYSTEM) */
                                 dwErrorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                 pBuffer,
                                 cchBufferLength,
                                 NULL);
    return (cchMsg > 0);
}