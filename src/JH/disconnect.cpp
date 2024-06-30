#include "player.h"
#include "general.h"

extern JH_player *players[];

void JH_player::onDisconnect()
{
    players[clientNum] = NULL;
    cleanSaves();
    delete(this);
}