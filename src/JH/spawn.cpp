#include "player.h"
#include "general.h"

extern JH_player *players[];

void JH_player::spawnPlayer()
{
    int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_SPAWN], 0);
    Scr_FreeThread(threadId);
    onSpawn();
}

bool JH_player::canSpawn()
{
    if(players[clientNum]->state == PLAYERSTATE_PLAYING || players[clientNum]->state == PLAYERSTATE_PAUSED)
        return true;
    iprintln("^3Cannot spawn right now");
    return false;
}

void JH_player::onSpawn()
{
    isElevating = false;
    couldBounce = false;
    wasOnGround = false;
    backwardsCount = 0;
    oldVelocity[0] = 0;
    oldVelocity[1] = 0;
    oldVelocity[2] = 0;
    jumpStartOriginSet = false;
    nextFrame = NEXTFRAME_NONE;
    rpgTime = 0;
    bounceTime = 0;
    jumpTime = 0;
    RPGCountJump = 0;
    drawCheckpoints();
    resumeRun();
    FPSReset();
}

bool JH_player::canSpectate()
{
    if(players[clientNum]->state == PLAYERSTATE_PLAYING || players[clientNum]->state == PLAYERSTATE_PAUSED)
        return true;
    iprintln("^3Cannot spectate right now");
    return false;
}

void JH_player::spawnSpectator()
{
    int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_SPECTATE], 0);
    Scr_FreeThread(threadId);
    onSpectate();
}

void JH_player::onSpectate()
{
    hideCheckpoints();
    pauseRun();
}