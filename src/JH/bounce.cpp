#include "player.h"
#include "general.h"

void JH_player::onBounce()
{
    iprintln("bounced");
    bounceTime = ent->client->sess.cmd.serverTime;
    run.bounceCount++;
    if(rpgTime > bounceTime - 500)
    {
        char str[MAX_STRING_CHARS];
        snprintf(str, MAX_STRING_CHARS - 1, "Early RPG by %d", bounceTime - rpgTime);
        iprintln(str);
    }
}