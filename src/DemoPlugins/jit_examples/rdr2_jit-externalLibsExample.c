#include <GlumityLib/GlumityLib.h>

// Example of adding a library from the env's libs folder:
// ADD_LIB(kernel32)
// they have to be added after "GlumityLib.h" or you define your own ADD_LIB(x)

/* ScriptHook C port: main.h (scriptRegister, WAIT, ...) + natives.h (all natives).
   Adjust the path to wherever the headers live under tcc_include\include. 
   == Small external lib calling example ==*/

// Macro derived from GlumityLib.h
// Isn't handled by the preprocessor but read from the bridge to decide which libs to link
// user32 is needed in this to call MessageBoxA
ADD_LIB(user32)

#include <Scipthookrdr2/main.h>
#include <Scipthookrdr2/natives.h>

#define MY_PLUGIN "rdr2_jit-extLibExample"

void MyScript()
{
    MessageBoxA(NULL, "Hello from MessageBoxA :)", "HI!", MB_OK);
    while (1)
    {
        // Empty script, ideally not what you do but this is just a demo
        WAIT(0);
    }
}

GLUMITYV2_RDR2_PLUGIN_ENTRY
{
    scriptRegister(bridgeInstance, MyScript);
}

GLUMITYV2_RDR2_PLUGIN_EXIT
{
    scriptUnregister(bridgeInstance);
    return 1;
}