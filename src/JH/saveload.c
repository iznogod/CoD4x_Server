#include "saveload.h"

extern JH_PLAYER jh_players[];

void JH_saveload_loadSave(int clientNum, JH_SAVE *save);

void JH_saveload_clearSaves(int clientNum)
{
    JH_SAVE *save = jh_players[clientNum].save;
    while (save != NULL)
    {
        JH_SAVE *prevSave = save->prevSave;
        free(save);
        save = prevSave;
    }
    jh_players[clientNum].save = NULL;
}

void JH_saveload_save(int entnum)
{
    gentity_t *entity = &g_entities[entnum];
    if(entity->client->sess.sessionState != SESS_STATE_PLAYING)
    {
        //player not alive
        return;
    }
    if(jh_players[entnum].playerState != PLAYERSTATE_PLAYING && jh_players[entnum].playerState != PLAYERSTATE_PAUSED)
    {
        //player not allowed to save
        JH_util_iprintln(entnum, "^1Cannot save right now");
        return;
    }
    jh_players[entnum].backwardsCount = 0;
    if(entity->client->ps.groundEntityNum == 1023)
    {
        //not on ground
        JH_util_iprintln(entnum, "^1Cannot save in air");
        return;
    }
    JH_SAVE *save = (JH_SAVE *)malloc(sizeof(JH_SAVE));
    if (save == NULL)
    {
        JH_util_iprintln(entnum, "^1Cannot create save");
        return;
    }
    save->checkpoint = jh_players[entnum].checkpoint;
    save->doubleRPGCount = jh_players[entnum].run.doubleRPGCount;
    save->FPSState = jh_players[entnum].run.FPSState;
    save->RPGCount = jh_players[entnum].run.RPGCount;
    save->runFlags = jh_players[entnum].run.runFlags_ever;
    save->prevSave = jh_players[entnum].save;
    jh_players[entnum].save = save;
    VectorCopy(g_entities[entnum].client->ps.origin, save->origin);
    VectorCopy(g_entities[entnum].client->ps.viewangles, save->angles);
    JH_util_iprintln(entnum, "^2Position saved");
    JH_statistics_onSave(entnum);
}

void JH_saveload_load(int entnum)
{
    gentity_t *entity = &g_entities[entnum];
    if(entity->client->sess.sessionState != SESS_STATE_PLAYING)
    {
        //player not alive
        return;
    }
    if(jh_players[entnum].playerState != PLAYERSTATE_PLAYING && jh_players[entnum].playerState != PLAYERSTATE_PAUSED)
    {
        //player not allowed to load
        JH_util_iprintln(entnum, "^1Cannot load right now");
        return;
    }
    JH_SAVE *save = jh_players[entnum].save;
    int backwardsCount = jh_players[entnum].backwardsCount;
    while(backwardsCount > 0 && save != NULL)
    {
        save = save->prevSave;
        backwardsCount--;
    }
    if(save == NULL)
    {
        JH_util_iprintln(entnum, "^1Failed loading position");
    }
    else
    {
        JH_saveload_loadSave(entnum, save);
        gentity_t *ent = SV_GentityNum(entnum);
        int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_LOADPOSITION], 0);
        Scr_FreeThread(threadId);
        JH_util_iprintln(entnum, "^2Position loaded");
        JH_statistics_load(entnum);
    }
}

void JH_saveload_loadSave(int clientNum, JH_SAVE *save)
{
    // sets origin, angles
    // resets pm_flags and velocity
    // keeps stance
    // todo: add rpg count etc

    extern void SV_UnlinkEntity(struct gentity_s * gEnt);
    extern void Pmove(struct pmove_t *);

// identical for cod2 and cod4
#define PMF_DUCKED 0x2
#define PMF_PRONE 0x1
#define EF_TELEPORT_BIT 0x2

    struct gentity_s *ent = &g_entities[clientNum];

    bool isUsingTurret;
#ifdef COD2
    isUsingTurret = ((ent->client->ps.pm_flags & 0x800000) != 0 && (ent->client->ps.eFlags & 0x300) != 0);
#else
    isUsingTurret = ((ent->client->ps.otherFlags & 4) != 0 && (ent->client->ps.eFlags & 0x300) != 0);
#endif

    // stop using MGs
    if (isUsingTurret)
    {
        G_ClientStopUsingTurret(&g_entities[ent->client->ps.viewlocked_entNum]);
    }

    G_EntUnlink(ent);

    // unlink client from linkto() stuffs

    if (ent->r.linked)
    {
        SV_UnlinkEntity(ent);
    }

    // clear flags
    ent->client->ps.pm_flags &= (PMF_DUCKED | PMF_PRONE); // keep stance
    ent->client->ps.eFlags ^= EF_TELEPORT_BIT;            // alternate teleport flag, unsure why

    // set times
    ent->client->ps.pm_time = 0;
    ent->client->ps.jumpTime = 0; // to reset wallspeed effects

    // set origin
    VectorCopy(save->origin, ent->client->ps.origin);
    G_SetOrigin(ent, save->origin);

    // reset velocity
    ent->client->ps.velocity[0] = 0;
    ent->client->ps.velocity[1] = 0;
    ent->client->ps.velocity[2] = 0;

#ifndef COD2
    ent->client->ps.sprintState.sprintButtonUpRequired = 0;
    ent->client->ps.sprintState.sprintDelay = 0;
    ent->client->ps.sprintState.lastSprintStart = 0;
    ent->client->ps.sprintState.lastSprintEnd = 0;
    ent->client->ps.sprintState.sprintStartMaxLength = 0;
#endif

    // pretend we're not proning so that prone angle is ok after having called SetClientViewAngle (otherwise it gets a correction)
    int flags = ent->client->ps.pm_flags;
    ent->client->ps.pm_flags &= ~PMF_PRONE;
    ent->client->sess.cmd.serverTime = level.time; // if this isnt set then errordecay takes place

    SetClientViewAngle(ent, save->angles);

    // create a pmove object and execute to bypass the errordecay thing
    struct pmove_t pm;
    memset(&pm, 0, sizeof(pm));
    pm.ps = &ent->client->ps;
    memcpy(&pm.cmd, &ent->client->sess.cmd, sizeof(pm.cmd));
    pm.cmd.forwardmove = 0;
    pm.cmd.rightmove = 0;
    pm.cmd.serverTime = level.time - 50;

    ent->client->sess.oldcmd.serverTime = level.time - 100;
    pm.oldcmd = ent->client->sess.oldcmd;
    pm.tracemask = 42008593;
    pm.handler = 1;
    Pmove(&pm);

    // reset velocity
    ent->client->ps.velocity[0] = 0;
    ent->client->ps.velocity[1] = 0;
    ent->client->ps.velocity[2] = 0;

    // restore prone if any
    ent->client->ps.pm_flags = flags;

    SV_LinkEntity(ent);

    JH_checkpoints_setCheckpoint(clientNum, save->checkpoint);
    jh_players[clientNum].run.doubleRPGCount = save->doubleRPGCount;
    jh_players[clientNum].run.FPSState = save->FPSState;
    jh_players[clientNum].run.RPGCount = save->RPGCount;
    jh_players[clientNum].run.runFlags_ever = save->runFlags;
    jh_players[clientNum].jumpStartOriginSet = false;
}
