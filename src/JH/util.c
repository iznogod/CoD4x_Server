#include "util.h"

void JH_printf();

void JH_util_addFunctions()
{
    Scr_AddFunction("getcvar", GScr_GetCvar, 0);
	Scr_AddFunction("setcvar", GScr_SetCvar, 0);
    Scr_AddFunction("printf", JH_printf, 0);
}

void JH_util_addMethods()
{
    Scr_AddMethod("setclientcvar", PlayerCmd_SetClientDvar, 0);
}

void JH_printf()
{
  char *str = Scr_GetString(0);
  Com_Printf(CON_CHANNEL_DONT_FILTER, str);
}

void JH_util_iprintln(int clientNum, char *msg)
{
    SV_GameSendServerCommand(clientNum, 0, va("%c \"%s\"", 'f', msg));
}