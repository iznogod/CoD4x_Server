#include "player.h"
#include "general.h"

extern JH_player *players[];

void jh_connect_onConnected(scr_entref_t ent);

void JH_connect_addMethods()
{
    Scr_AddMethod("jh_onconnected", jh_connect_onConnected, qfalse);
}

void jh_connect_onConnected(scr_entref_t ent)
{
    if(players[ent.entnum] != NULL)
    {
        players[ent.entnum]->hideHUD();
    }
}

void JH_player::onConnect(int _clientNum)
{
    if(players[_clientNum] != NULL)
    {
        players[_clientNum]->cleanSaves();
        delete(players[_clientNum]);
    }
    players[_clientNum] = this;
    ent = &g_entities[_clientNum];
    clientNum = _clientNum;
    checkpoint = NULL;
    save = NULL;
    run.state = RUNSTATE_NONE;
    state = PLAYERSTATE_PLAYING;
    nextFrame = NEXTFRAME_NONE;
    oldHUDstr[0] = '\0';
    hudState = HUDSTATE_UNDEFINED;
    FPSReset();
    resetSettings();
}