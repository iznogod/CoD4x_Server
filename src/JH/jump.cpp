#include "player.h"
#include "general.h"

void JH_player::onJump(vec3_t origin)
{
    jumpTime = ent->client->sess.cmd.serverTime;
    VectorCopy(origin, jumpStartOrigin);
    jumpStartOriginSet = true;
    if(rpgTime > jumpTime - 500)
    {
        char str[MAX_STRING_CHARS];
        snprintf(str, MAX_STRING_CHARS - 1, "Early RPG by %d", jumpTime - rpgTime);
        iprintln(str);
    }
}

void JH_player::onLand()
{
    RPGCountJump = 0;
    if(jumpStartOriginSet)
    {
        vec3_t diff;
        diff[0] = fabs(ent->client->ps.origin[0] -  jumpStartOrigin[0]) + 30;
        diff[1] = fabs(ent->client->ps.origin[1] -  jumpStartOrigin[1]) + 30;
        diff[2] = fabs(ent->client->ps.origin[2] -  jumpStartOrigin[2]);

        float gap = sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
        
        char str[MAX_STRING_CHARS];
        snprintf(str, MAX_STRING_CHARS - 1, "Jumped (%.2f, %.2f, %.2f), gap %.2f", diff[0], diff[1], diff[2], gap);
        iprintln(str);

        jumpStartOriginSet = false;
    }
}