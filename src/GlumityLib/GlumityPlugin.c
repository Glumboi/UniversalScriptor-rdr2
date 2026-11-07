#include "GlumityPlugin.h"

void GlumityPlugin_printf(const char *fmt, const char *printHeaderInner, ...)
{
    printf("[%s]: ", printHeaderInner);

    va_list args;
    va_start(args, printHeaderInner);
    vprintf(fmt, args);
    va_end(args);
}

void GetPluginDirectory(HMODULE hModule, char *outBuffer, DWORD bufferSize)
{
    GetModuleFileNameA(hModule, outBuffer, bufferSize);
    char *lastSlash = strrchr(outBuffer, '\\');
    if (lastSlash)
    {
        *(lastSlash + 1) = '\0'; 
    }
}