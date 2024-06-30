#include "player.h"
#include "general.h"

extern JH_player *players[];

void JH_Callback_PlayerConnect(int clientNum)
{
    JH_player *player = new JH_player();
    player->onConnect(clientNum);
}

void JH_Callback_PlayerKilled(int clientNum)
{
    if(players[clientNum] != NULL)
    {
        players[clientNum]->onKilled();
    }
}

void JH_player::onKilled()
{
    hideHUD();
    hideCheckpoints();
}

void JH_Callback_AddFunctions()
{
    JH_checkpoints_addFunctions();
    JH_util_addFunctions();
    JH_mysql_addFunctions();
}

void JH_Callback_UserInfoChanged(int clientNum)
{
    char *fpsstring = Info_ValueForKey(svs.clients[clientNum].userinfo, "com_maxfps");
    int fps = atoi(fpsstring);
    if(players[clientNum] != NULL && fps > 0 && fps <= 1000)
    {
        players[clientNum]->onFPSUserinfo(1000 / fps);
    }
}

void JH_Callback_AddMethods()
{
    JH_util_addMethods();
    JH_clientCommand_addMethods();
    JH_saves_addMethods();
    JH_runs_addMethods();
    JH_fps_addMethods();
    JH_connect_addMethods();
}

void JH_Callback_ClientEndFrame(gentity_t *ent)
{
  int clientNum = ent - g_entities;
  client_t *client = &svs.clients[clientNum];
  
  if(client->state == CS_ACTIVE && players[clientNum] != NULL)
  {
    JH_nextFrame action = players[clientNum]->nextFrame;
    players[clientNum]->nextFrame = NEXTFRAME_NONE;
    switch(action)
    {
      case NEXTFRAME_LOAD:
      {
        if(players[clientNum]->canLoad())
            players[clientNum]->loadPosition();
        break;
      }
      case NEXTFRAME_SPAWN:
      {
        if(players[clientNum]->canSpawn())
            players[clientNum]->spawnPlayer();
        break;
      }
      case NEXTFRAME_SPECTATE:
      {
        if(players[clientNum]->canSpectate())
            players[clientNum]->spawnSpectator();
        break;
      }
      default:
      {
        break;
      }
    }
    players[clientNum]->updateHUD(false);
  }
  ClientEndFrame(ent);
}

void JH_Callback_Jump(int clientNum, vec3_t origin)
{
    if(players[clientNum] != NULL)
        players[clientNum]->onJump(origin);
}

void JH_Callback_RPG(gentity_t *player, gentity_t *rpg)
{
    int clientNum = player - g_entities;
    if(players[clientNum] != NULL)
        players[clientNum]->onRPGShot(rpg);
}

void JH_Callback_BeforeClientThink(client_t *client, usercmd_t *ucmd)
{
    int clientNum = client - svs.clients;
    if(players[clientNum] != NULL)
    {
        players[clientNum]->beforeClientThink(ucmd);
    }
}

void JH_player::beforeClientThink(usercmd_t *ucmd)
{
    VectorCopy(ent->client->ps.velocity, oldVelocity);
    if(fpsFix)
    {
        char *fpsstring = Info_ValueForKey(svs.clients[clientNum].userinfo, "com_maxfps");
        int fps = atoi(fpsstring);
        if(fps > 0 && fps <= 1000)
        {
            int actualFrameTime = ucmd->serverTime - prevTime;
            int wantedFrameTime = 1000 / fps;
            if(actualFrameTime != wantedFrameTime && abs(actualFrameTime - wantedFrameTime) < 10 && abs(actualFrameTime - wantedFrameTime) <= actualFrameTime / 2)
            {
                // adjust frametime
                svs.clients[clientNum].lastUsercmd.serverTime = svs.clients[clientNum].lastUsercmd.serverTime - actualFrameTime + wantedFrameTime;
                ucmd->serverTime = ucmd->serverTime - actualFrameTime + wantedFrameTime;
            }
        }
    }
}

void JH_Callback_AfterClientThink(client_t *client, usercmd_t *ucmd)
{
    int clientNum = client - svs.clients;
    if(players[clientNum] != NULL)
        players[clientNum]->afterClientThink(ucmd);
}

void JH_Callback_Elevate_End(struct pmove_t *pm)
{
    if(players[pm->ps->clientNum] != NULL)
        players[pm->ps->clientNum]->isElevating = false;
}

void JH_player::afterClientThink(usercmd_t *ucmd)
{
    if (ent->client->sess.sessionState == SESS_STATE_PLAYING && (state == PLAYERSTATE_PLAYING || state == PLAYERSTATE_PAUSED))
    {
        bool bounced = !canBounce() && couldBounce && !isOnGround();
        couldBounce = canBounce();

        if(bounced)
            onBounce();
        
        bool landed = isOnGround() && !wasOnGround;
        wasOnGround = isOnGround();

        if(landed)
            onLand();
    
        if(!halfBeat && !isOnGround())
        {
            applyAntiHalfbeat(ucmd);
        }
        if(autoRPG && bounced && !isOnGround())
        {
            applyAutoRPG(ucmd);
        }
        if(memeMode)
            applyMemeMode(bounced);

        FPSThink(ucmd->serverTime);
        checkpointThink();
    }
    prevTime = ucmd->serverTime;
}

void JH_player::applyMemeMode(bool bounced)
{
    if(!isOnGround())
    {
        if(fabs(ent->client->ps.velocity[0]) > 250 && fabs(ent->client->ps.velocity[0]) > fabs(oldVelocity[0]))
            ent->client->ps.velocity[0] = (2 * ent->client->ps.velocity[0] - oldVelocity[0]);
        if(fabs(ent->client->ps.velocity[1]) > 250 && fabs(ent->client->ps.velocity[1]) > fabs(oldVelocity[1]))
            ent->client->ps.velocity[1] = (2 * ent->client->ps.velocity[1] - oldVelocity[1]);
        if(bounced)
        {
            ent->client->ps.velocity[2] += 0.1 * (ent->client->ps.velocity[2] - oldVelocity[2]);
        }
    }
}

void JH_player::applyAntiHalfbeat(usercmd_t *ucmd)
{
    int frametime = ucmd->serverTime - prevTime;
    int accel = round((float)(frametime * g_gravity->current.floatval) * 0.001);
    if(oldVelocity[2] - ent->client->ps.velocity[2] == accel)
    {
        //player is free-falling in air
        if(fabs(oldVelocity[0]) < fabs(ent->client->ps.velocity[0]) && fabs(ent->client->ps.velocity[0]) > 250 && ucmd->forwardmove == 0)
            ent->client->ps.velocity[0] = oldVelocity[0];
        if(fabs(oldVelocity[1]) < fabs(ent->client->ps.velocity[1]) && fabs(ent->client->ps.velocity[1]) > 250 && ucmd->forwardmove == 0)
            ent->client->ps.velocity[1] = oldVelocity[1];
    }
}

void JH_player::applyAutoRPG(usercmd_t *ucmd)
{
    onRPG();
    ent->client->ps.velocity[0] -= cos(ent->client->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
    ent->client->ps.velocity[1] -= cos(ent->client->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
    ent->client->ps.velocity[2] += sin(85.0f/360.0f*2.0f*M_PI) * 64;
}

void JH_Callback_Elevate_Start(struct pmove_t *pm)
{
    int clientNum = pm->ps->clientNum;
    if(players[clientNum] != NULL)
    {
        if(!players[clientNum]->isElevating)
        {
            players[clientNum]->onElevate(pm);
            players[clientNum]->isElevating = true;
        }
    }
}

void JH_player::onElevate(pmove_t *pm)
{
    if(pm->ps->groundEntityNum != 1023 || pm->ps->oldVelocity[2] > 0)
    {
        //normal elevator
        iprintln("Normale ele detected");
    }
    else
    {
        //side elevator
        iprintln("Side ele detected");
    }
}

