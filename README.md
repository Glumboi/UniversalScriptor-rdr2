

# UniversalScriptor

A stripped down GlumityV2ToolSuite as a bridge to inject a small runtime tcc compiler. Technically compatible with everything!

# Demonstration 
![alt text](screenshots/screenshot1.png)

## jit script example

```C 
#include <GlumityLib/GlumityLib.h>

/* ScriptHook C port: main.h (scriptRegister, WAIT, ...) + natives.h (all natives).
   Adjust the path to wherever the headers live under tcc_include\include. */
#include <Scipthookrdr2/main.h>
#include <Scipthookrdr2/natives.h>

#define MY_PLUGIN "rdr2_jit-godmode"

void MyScript()
{
    while (1)
    {
        int loading = DLC_GET_IS_LOADING_SCREEN_ACTIVE();

        Player player = PLAYER_PLAYER_ID();
        Ped ped = PLAYER_PLAYER_PED_ID();
        int exists = ENTITY_DOES_ENTITY_EXIST(ped);

        if (!loading && exists)
        {
            /* god mode, every frame */
            ENTITY_SET_ENTITY_INVINCIBLE(ped, 1);
            PLAYER_SET_PLAYER_INVINCIBLE(player, 1);

            int maxHealth = PED_GET_PED_MAX_HEALTH(ped);
            if (ENTITY_GET_ENTITY_HEALTH(ped) < maxHealth)
                ENTITY_SET_ENTITY_HEALTH(ped, maxHealth, 0);
        }

        WAIT(0);
    }
}

void RdrScriptMain(HMODULE glumityInstance)
{
    scriptRegister(glumityInstance, MyScript);
}
```

<a href="https://www.nexusmods.com/reddeadredemption2/mods/11513?tab=description" target="_blank">
  <img src="https://img.shields.io/badge/Nexus%20Mods-View%20Mod-red?style=flat-square&logo=nexusmods" alt="Nexus Mods">
</a>

## Build requirements

1. Favourite code editor
2. Xmake (at least version ```3.0.1```) 
2. A C and C++ toolchain of choice, since this project is windows targetted, MSVC is recommended
3. Odin language install (at least version ````dev-2025-08-nightly:accdd7c````)

Running ```"buildAll.bat"``` inside root folder of repository should automatically build everything, the build scripts are relatively basic and barebones.  

Output binaries end up inside ```"UniversalScriptor\build"```. To use them, refer to [How to use built binaries](#how-to-use-built-binaries)
# **More documentation follows at some point** 
