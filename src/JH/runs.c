#include "runs.h"

void JH_runs_resetRun(scr_entref_t ent);
void JH_runs_startRun(scr_entref_t ent);

extern JH_PLAYER jh_players[];

void JH_runs_addMethods()
{
    Scr_AddMethod("jh_runs_resetrun", JH_runs_resetRun, qfalse);
    Scr_AddMethod("jh_runs_startrun", JH_runs_startRun, qfalse);
}

void JH_runs_resetRun(scr_entref_t ent)
{
    if((jh_players[ent.entnum].playerState == PLAYERSTATE_PAUSED || jh_players[ent.entnum].playerState == PLAYERSTATE_PLAYING) && jh_players[ent.entnum].run.runState != RUNSTATE_INITIALIZING)
    {
        jh_players[ent.entnum].run.runState = RUNSTATE_INITIALIZING;
        Scr_AddInt(1);
    }
    else
    {
        Scr_AddInt(0);
    }
}

void JH_runs_startRun(scr_entref_t ent)
{
    if((jh_players[ent.entnum].playerState == PLAYERSTATE_PAUSED || jh_players[ent.entnum].playerState == PLAYERSTATE_PLAYING) && jh_players[ent.entnum].run.runState == RUNSTATE_INITIALIZING)
    {
        jh_players[ent.entnum].run.runState = RUNSTATE_PLAYING;

        //spawn
        //init other vars
        Scr_AddInt(1);
    }
    else
    {
        Scr_AddInt(0);
    }
}