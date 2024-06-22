#include "saveload.h"

void JH_saveClearPositions(int entnum);
void JH_saveCreate(scr_entref_t ent);
void JH_saveSetPosition(scr_entref_t ent);
void JH_saveSetAngles(scr_entref_t ent);
void JH_saveSelect(scr_entref_t ent);
void JH_saveGetPosition(scr_entref_t ent);
void JH_saveGetAngles(scr_entref_t ent);
void JH_setOriginAndAngles(scr_entref_t entref);

JH_SAVE *jh_saves[MAX_CLIENTS] = {0};
JH_SAVE *jh_selected_saves[MAX_CLIENTS] = {0};

void JH_saveLoad_addMethods()
{
    Scr_AddMethod("savecreate", JH_saveCreate, qfalse);
    Scr_AddMethod("savesetposition", JH_saveSetPosition, qfalse);
    Scr_AddMethod("savesetangles", JH_saveSetAngles, qfalse);
    Scr_AddMethod("saveselect", JH_saveSelect, qfalse);
    Scr_AddMethod("savegetposition", JH_saveGetPosition, qfalse);
    Scr_AddMethod("savegetangles", JH_saveGetAngles, qfalse);
    Scr_AddMethod("setoriginandangles", JH_setOriginAndAngles, qfalse);
}

void JH_saveClearPositions(int entnum)
{
    JH_SAVE *save = jh_saves[entnum];
    while (save != NULL)
    {
        JH_SAVE *prevSave = save->prevSave;
        free(save);
        save = prevSave;
    }
    jh_saves[entnum] = NULL;
    jh_selected_saves[entnum] = NULL;
}

void JH_saveCreate(scr_entref_t ent)
{
    JH_SAVE *save = (JH_SAVE *)malloc(sizeof(JH_SAVE));
    if (save == NULL)
    {
        Scr_AddInt(0);
        return;
    }
    save->prevSave = jh_saves[ent.entnum];
    jh_saves[ent.entnum] = save;
    Scr_AddInt(1);
}

void JH_saveSetPosition(scr_entref_t ent)
{
    if (jh_saves[ent.entnum] != NULL)
    {
        Scr_GetVector(0, jh_saves[ent.entnum]->origin);
    }
}

void JH_saveSetAngles(scr_entref_t ent)
{
    if (jh_saves[ent.entnum] != NULL)
    {
        Scr_GetVector(0, jh_saves[ent.entnum]->angles);
    }
}

void JH_saveSelect(scr_entref_t ent)
{
    int backwardsCount = Scr_GetInt(0);
    jh_selected_saves[ent.entnum] = jh_saves[ent.entnum];
    if (jh_selected_saves[ent.entnum] == NULL)
    {
        Scr_AddInt(0);
        return;
    }
    while (backwardsCount > 0)
    {
        jh_selected_saves[ent.entnum] = jh_selected_saves[ent.entnum]->prevSave;
        if (jh_selected_saves[ent.entnum] == NULL)
        {
            Scr_AddInt(0);
            return;
        }
        backwardsCount--;
    }
    Scr_AddInt(1);
}
void JH_saveGetPosition(scr_entref_t ent)
{
    if (jh_selected_saves[ent.entnum] != NULL)
    {
        Scr_AddVector(jh_selected_saves[ent.entnum]->origin);
    }
}

void JH_saveGetAngles(scr_entref_t ent)
{
    if (jh_selected_saves[ent.entnum] != NULL)
    {
        Scr_AddVector(jh_selected_saves[ent.entnum]->angles);
    }
}

void JH_setOriginAndAngles(scr_entref_t entref)
{
    vec3_t origin;
    vec3_t angles;
    Scr_GetVector(0, origin);
    Scr_GetVector(1, angles);

    // sets origin, angles
    // resets pm_flags and velocity
    // keeps stance

    extern void SV_UnlinkEntity(struct gentity_s * gEnt);
    extern void Pmove(struct pmove_t *);

// identical for cod2 and cod4
#define PMF_DUCKED 0x2
#define PMF_PRONE 0x1
#define EF_TELEPORT_BIT 0x2

    struct gentity_s *ent;
    ent = &g_entities[entref.entnum];

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
    VectorCopy(origin, ent->client->ps.origin);
    G_SetOrigin(ent, origin);

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

    SetClientViewAngle(ent, angles);

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
}
