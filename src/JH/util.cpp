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

bool JH_player::isOnGround()
{
    return ent->client->ps.groundEntityNum != 1023;
}
void JH_player::iprintln(const char *str)
{
    SV_GameSendServerCommand(clientNum, 0, va("%c \"%s\"", 'f', str));
}

bool JH_player::canBounce()
{
    return ((ent->client->ps.pm_flags & 0x4000) != 0);
}

void JH_player::setClientCvar(svscmd_type rel, const char *cvar, const char *str)
{
    SV_GameSendServerCommand(clientNum, rel, va("%c %s \"%s\"", 'v', cvar, str));
}