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