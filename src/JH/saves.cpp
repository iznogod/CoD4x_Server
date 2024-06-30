#include "player.h"
#include "general.h"

extern JH_player *players[];

void JH_clientcommand_load(scr_entref_t entref)
{
    players[entref.entnum]->nextFrame = NEXTFRAME_LOAD;
    int backwardsCount = Scr_GetInt(0);
    if(backwardsCount == 0)
        players[entref.entnum]->backwardsCount = 0;
    else
        players[entref.entnum]->backwardsCount += backwardsCount;
}

void JH_clientcommand_save(scr_entref_t entref)
{
    if(players[entref.entnum]->canSave())
        players[entref.entnum]->savePosition();
}

void JH_saves_addMethods()
{
    Scr_AddMethod("jh_load", JH_clientcommand_load, qfalse);
    Scr_AddMethod("jh_save", JH_clientcommand_save, qfalse);
}

void JH_player::cleanSaves()
{
    JH_save *_save = save;
    while(_save != NULL)
    {
        JH_save *prevSave = _save->prevSave;
        delete(_save);
        _save = prevSave;
    }
    save = NULL;
}

bool JH_player::canSave()
{
    if(ent->client->sess.sessionState != SESS_STATE_PLAYING)
    {
        return false;
    }
    if(state != PLAYERSTATE_PLAYING && state != PLAYERSTATE_PAUSED)
    {
        iprintln("^1Cannot save right now");
        return false;
    }
    if(!isOnGround())
    {
        iprintln("^1Cannot save in air");
        return false;
    }
    return true;
}

void JH_player::savePosition()
{
    JH_save *_save = new JH_save(save);
    if(_save == NULL)
    {
        iprintln("^1Cannot create save");
        return;
    }
    save = _save;
    _save->checkpoint = checkpoint;
    _save->RPGCount = run.RPGCount;
    _save->doubleRPGCount = run.doubleRPGCount;
    _save->fps.fpsFlags = fps.fpsFlags;
    _save->fps.frameTimeUserinfo = fps.frameTimeUserinfo;
    VectorCopy(ent->client->ps.origin, _save->origin);
    VectorCopy(ent->client->ps.viewangles, _save->angles);
    run.saveCount++;
    iprintln("^2Position saved");
    backwardsCount = 0;
}

extern "C"
{
void interface_unlinkEntity(gentity_t *ent)
{
    extern void SV_UnlinkEntity(struct gentity_s * gEnt);
    SV_UnlinkEntity(ent);
}

void interface_Pmove(pmove_t *pm)
{
    extern void Pmove(pmove_t *);
    Pmove(pm);
}
}

bool JH_player::canLoad()
{
    if(ent->client->sess.sessionState != SESS_STATE_PLAYING)
    {
        return false;
    }
    if(state != PLAYERSTATE_PLAYING && state != PLAYERSTATE_PAUSED)
    {
        iprintln("^1Cannot load right now");
        return false;
    }
    return true;
}

void JH_player::setOriginAndAngles(vec3_t origin, vec3_t angles)
{

// identical for cod2 and cod4
#define PMF_DUCKED 0x2
#define PMF_PRONE 0x1
#define EF_TELEPORT_BIT 0x2

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
        interface_unlinkEntity(ent);
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
    interface_Pmove(&pm);

    // reset velocity
    ent->client->ps.velocity[0] = 0;
    ent->client->ps.velocity[1] = 0;
    ent->client->ps.velocity[2] = 0;

    // restore prone if any
    ent->client->ps.pm_flags = flags;

    SV_LinkEntity(ent);
}

void JH_player::loadPosition()
{
    JH_save *_save = save;
    int _backwardsCount = backwardsCount;
    while(_backwardsCount > 0 && _save != NULL)
    {
        _save = _save->prevSave;
        _backwardsCount--;
    }
    if(_save == NULL)
    {
        iprintln("^1Failed loading position");
        return;
    }
    setCheckpoint(_save->checkpoint);
    setOriginAndAngles(_save->origin, _save->angles);
    int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_LOADPOSITION], 0);
    Scr_FreeThread(threadId);
    run.RPGCount = _save->RPGCount;
    run.doubleRPGCount = _save->doubleRPGCount;
    run.loadCount++;
    setFPS(_save->fps);
    //setFlags(_save->flags);
    jumpStartOriginSet = false;
    iprintln("^2Position loaded");
}