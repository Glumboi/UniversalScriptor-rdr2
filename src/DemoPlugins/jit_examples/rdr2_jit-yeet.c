#include <GlumityLib/GlumityLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ScriptHook C port: main.h (scriptRegister, WAIT, ...) + natives.h (all natives).
   Adjust the path to wherever the headers live under tcc_include\include. */
#include <Scipthookrdr2/main.h>
#include <Scipthookrdr2/natives.h>

#define MY_PLUGIN "rdr2_jit-yeet"

#define INI_FILE "Yeet.ini"

struct
{
    float forwardForce;
    float upForce;
} g_config;

void InitConfig(void)
{
    FILE *file;

    g_config.forwardForce = 15.0f;
    g_config.upForce = 5.0f;

    file = fopen(INI_FILE, "r");

    if (file != NULL)
    {
        char line[256];
        int in_settings_section = 0;
        int found_forward = 0;
        int found_up = 0;

        while (fgets(line, sizeof(line), file) != NULL)
        {
            line[strcspn(line, "\r\n")] = '\0';

            if (strcmp(line, "[settings]") == 0)
            {
                in_settings_section = 1;
                continue;
            }
            else if (line[0] == '[')
            {
                in_settings_section = 0;
                continue;
            }

            if (in_settings_section)
            {
                char key[64];
                char val[64];
                if (sscanf(line, " %63[^=] = %63s", key, val) == 2)
                {
                    key[strcspn(key, " \t")] = '\0';

                    if (strcmp(key, "forwardForce") == 0)
                    {
                        g_config.forwardForce = (float)atof(val);
                        found_forward = 1;
                    }
                    else if (strcmp(key, "upForce") == 0)
                    {
                        g_config.upForce = (float)atof(val);
                        found_up = 1;
                    }
                }
            }
        }
        fclose(file);

        if (found_forward && found_up)
        {
            return;
        }
    }

    file = fopen(INI_FILE, "w");
    if (file != NULL)
    {
        fprintf(file, "[settings]\n");
        fprintf(file, "forwardForce=15.0\n");
        fprintf(file, "upForce=5.0\n");
        fclose(file);
    }

    g_config.forwardForce = 15.0f;
    g_config.upForce = 5.0f;
}

uint8_t promptCreated = 0;
Prompt throwPrompt = 0;

void CreateThrowPrompt()
{
    if (promptCreated || HUD__UI_PROMPT_IS_VALID(throwPrompt))
        return;

    throwPrompt = HUD__UI_PROMPT_REGISTER_BEGIN();
    HUD__UI_PROMPT_SET_CONTROL_ACTION(throwPrompt, 0x760A9C6F); // INPUT_CONTEXT_SECONDARY (G)

    HUD__UI_PROMPT_SET_TEXT(throwPrompt,
        MISC_VAR_STRING(10, 2, (UINT64)(uintptr_t)"LITERAL_STRING", (UINT64)(uintptr_t)"Yeet"));

    HUD__UI_PROMPT_SET_PRIORITY(throwPrompt, 3);
    HUD__UI_PROMPT_SET_TRANSPORT_MODE(throwPrompt, 0);
    HUD__UI_PROMPT_SET_STANDARD_MODE(throwPrompt, 1);
    HUD__UI_PROMPT_SET_ENABLED(throwPrompt, 1);
    HUD__UI_PROMPT_SET_VISIBLE(throwPrompt, 1);
    HUD__UI_PROMPT_REGISTER_END(throwPrompt);

    promptCreated = 1;
}

void YeetPedFromPlayer(Ped player, Entity target)
{
    PED_DETACH_CARRIABLE_ENTITY(target, 0, 1);
    TASK_CLEAR_PED_TASKS(player, 1, 0);
    ENTITY_SET_ENTITY_DYNAMIC(target, 1);
    PHYSICS_ACTIVATE_PHYSICS(target);
    WAIT(0);
    Vector3 forward = ENTITY_GET_ENTITY_FORWARD_VECTOR(player);
    ENTITY_APPLY_FORCE_TO_ENTITY_CENTER_OF_MASS(
        target, 1,
        forward.x * g_config.forwardForce,
        forward.y * g_config.forwardForce,
        g_config.upForce,
        0, 0, 1, 0);

    GLUMITY_PRINT_COLOR(CON_CYAN, "Yeeted entity %d\n", MY_PLUGIN, (int)target);
}

void DeleteThrowPrompt()
{
    if (!promptCreated)
        return;
    HUD__UI_PROMPT_DELETE(throwPrompt);
    throwPrompt = 0;
    promptCreated = 0;
}

void MyScript()
{
    InitConfig();
    GLUMITY_PRINT_COLOR(CON_GREEN, "Yeet loaded: forwardForce=%.1f upForce=%.1f\n",
        MY_PLUGIN, g_config.forwardForce, g_config.upForce);

    while (1)
    {
        WAIT(5);

        if (DLC_GET_IS_LOADING_SCREEN_ACTIVE())
            continue;

        Ped playerPed = PLAYER_PLAYER_PED_ID();
        if (!ENTITY_DOES_ENTITY_EXIST(playerPed))
            continue;

        if (PED_IS_PED_CARRYING_SOMETHING(playerPed))
        {
            Entity carriedEntity = PED__GET_FIRST_ENTITY_PED_IS_CARRYING(playerPed);
            if (!ENTITY_DOES_ENTITY_EXIST(carriedEntity))
                continue;

            int carriedPromptGroup = HUD__UI_PROMPT_GET_GROUP_ID_FOR_TARGET_ENTITY(carriedEntity);
            if (!promptCreated)
                CreateThrowPrompt();

            HUD__UI_PROMPT_SET_GROUP(throwPrompt, carriedPromptGroup, 0);
            HUD__UI_PROMPT_SET_ENABLED(throwPrompt, 1);
            HUD__UI_PROMPT_SET_VISIBLE(throwPrompt, 1);

            if (HUD__UI_PROMPT_HAS_STANDARD_MODE_COMPLETED(throwPrompt, 0))
            {
                YeetPedFromPlayer(playerPed, carriedEntity);

                HUD__UI_PROMPT_SET_ENABLED(throwPrompt, 0);
                HUD__UI_PROMPT_SET_VISIBLE(throwPrompt, 0);
            }
        }
        else
        {
            DeleteThrowPrompt();
        }
    }
}

void RdrScriptMain(HMODULE glumityInstance)
{
    scriptRegister(glumityInstance, MyScript);
}