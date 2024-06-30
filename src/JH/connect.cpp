#include "player.h"
#include "general.h"

extern JH_player *players[];

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
    isElevating = false;
    couldBounce = false;
    wasOnGround = false;
    backwardsCount = 0;
    oldVelocity[0] = 0;
    oldVelocity[1] = 0;
    oldVelocity[2] = 0;
    jumpStartOriginSet = false;
    checkpoint = NULL;
    save = NULL;
    state = PLAYERSTATE_PLAYING;
    nextFrame = NEXTFRAME_NONE;
    rpgTime = 0;
    bounceTime = 0;
    jumpTime = 0;
    RPGCountJump = 0;
    hudState = HUDSTATE_OFF;
    FPSReset();
    resetSettings();
}