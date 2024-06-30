#include "player.h"
#include "general.h"

void JH_util_printf();

void JH_util_addFunctions()
{
    Scr_AddFunction("getcvar", GScr_GetCvar, qfalse);
	Scr_AddFunction("setcvar", GScr_SetCvar, qfalse);
    Scr_AddFunction("printf", JH_util_printf, qfalse);
}

void JH_util_addMethods()
{
    Scr_AddMethod("setclientcvar", PlayerCmd_SetClientDvar, qfalse);
}

void JH_util_printf()
{
  char *str = Scr_GetString(0);
  Com_Printf(CON_CHANNEL_DONT_FILTER, str);
}
