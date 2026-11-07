#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <windows.h>
#include <stdio.h>
#include "GlumityPlugin.h"

#define DEFAULT_PLUGINCAP 10 * sizeof(GlumityPlugin)

typedef struct
{
    GlumityPlugin *plugins;

    size_t pluginsCap;
    size_t pluginsCount;

    HMODULE thisDll;
} GlumityPluginLoader;

#endif