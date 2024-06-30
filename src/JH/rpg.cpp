#include "general.h"
#include "player.h"

void JH_player::onRPGShot(gentity_t *rpg)
{
    onRPG();

    Scr_AddString(BG_GetWeaponDef(rpg->s.weapon)->szInternalName);
    Scr_AddEntity(rpg);
    int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_RPGFIRE], 2);
    Scr_FreeThread(threadId);

    rpgTime = ent->client->lastServerTime;
    char str[MAX_STRING_CHARS];
    snprintf(str, MAX_STRING_CHARS - 1, "RPG angle: %.2f", ent->client->ps.viewangles[0]);
    iprintln(str);

    if(bounceTime > jumpTime && bounceTime > rpgTime - 500)
    {
      snprintf(str, MAX_STRING_CHARS - 1, "Late RPG by %d", rpgTime - bounceTime);
      iprintln(str);
    }
    else if(jumpTime > rpgTime - 500)
    {
        snprintf(str, MAX_STRING_CHARS - 1, "Late RPG by %d", rpgTime - jumpTime);
        iprintln(str);
    }
}

void JH_player::onRPG()
{
    run.RPGCount++;
    RPGCountJump++;
    if(RPGCountJump == 2)
        onDoubleRPG();
}

void JH_player::onDoubleRPG()
{
    run.doubleRPGCount++;
    iprintln("^3Double RPG detected");
}