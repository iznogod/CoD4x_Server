#include "player.h"
#include "general.h"

extern JH_player *players[];

void JH_runs_resetRun(scr_entref_t ent);
void JH_runs_initializeRun(scr_entref_t ent);

void JH_runs_addMethods()
{
    Scr_AddMethod("jh_runs_resetrun", JH_runs_resetRun, qfalse);
    Scr_AddMethod("jh_runs_initializerun", JH_runs_initializeRun, qfalse);
}

void JH_runs_resetRun(scr_entref_t ent)
{
    if(players[ent.entnum] != NULL)
        players[ent.entnum]->resetRun();
}

void JH_runs_initializeRun(scr_entref_t ent)
{
    if(players[ent.entnum] != NULL)
        players[ent.entnum]->intializeRun(Scr_GetInt(0));
}

void JH_player::pauseRun()
{
    if(state == PLAYERSTATE_PLAYING)
    {
        state = PLAYERSTATE_PAUSED;
        run.stopTime = svs.time;
    }
}

void JH_player::resumeRun()
{
    if(state == PLAYERSTATE_PAUSED)
    {
        state = PLAYERSTATE_PLAYING;
        run.startTime += svs.time - run.stopTime;
    }
}

void JH_player::intializeRun(int _runID)
{
    if(run.state == RUNSTATE_INITIALIZING)
    {
        run.runID = _runID;
        run.state = RUNSTATE_PLAYING;
        resetSettings();
        cleanSaves();
        setStartCheckpoint();
        spawnPlayer();
    }
}

void JH_player::resetRun()
{
    if(run.state == RUNSTATE_PLAYING || run.state == RUNSTATE_NONE || run.state == RUNSTATE_FINISHED)
    {
        run.state = RUNSTATE_INITIALIZING;
        short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_RESETRUN], 0);
        Scr_FreeThread(ret);
    }
}