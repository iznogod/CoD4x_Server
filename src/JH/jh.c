#include "jh.h"
JH_PLAYER jh_players[MAX_CLIENTS] = {0};

void JH_Callback_Jump(int clientNum, int serverTime, vec3_t origin)
{
  client_t *client = &svs.clients[clientNum];
  jh_players[clientNum].jumpTime = client->gentity->client->sess.cmd.serverTime;
  VectorCopy(origin, jh_players[clientNum].jumpStartOrigin);
  jh_players[clientNum].jumpStartOriginSet = true;

  if(jh_players[clientNum].RPGTime > client->gentity->client->sess.cmd.serverTime - 500)
  {
    char str[MAX_STRING_CHARS];
    snprintf(str, MAX_STRING_CHARS - 1, "Early RPG by %d", client->gentity->client->sess.cmd.serverTime - jh_players[clientNum].RPGTime);
    JH_util_iprintln(clientNum, str);
  }
}

void JH_Callback_RPG(gentity_t *player, gentity_t *rpg)
{
  int callback = script_CallBacks_new[SCR_CB_RPGFIRE];
  Scr_AddString(BG_GetWeaponDef(rpg->s.weapon)->szInternalName);
  Scr_AddEntity(rpg);
  int threadId = Scr_ExecEntThread(player, callback, 2);
  Scr_FreeThread(threadId);
  jh_players[player - g_entities].RPGTime = player->client->lastServerTime;
  client_t *client = &svs.clients[player - g_entities];
  if(jh_players[player - g_entities].bounceTime > jh_players[player - g_entities].jumpTime)
  {
    if(jh_players[player - g_entities].bounceTime > client->gentity->client->sess.cmd.serverTime - 500)
    {
      char str[MAX_STRING_CHARS];
      snprintf(str, MAX_STRING_CHARS - 1, "Late RPG by %d", client->gentity->client->sess.cmd.serverTime - jh_players[player - g_entities].bounceTime);
      JH_util_iprintln(player - g_entities, str);
    }
  }
  else
  {
    if(jh_players[player - g_entities].jumpTime > client->gentity->client->sess.cmd.serverTime - 500)
    {
      char str[MAX_STRING_CHARS];
      snprintf(str, MAX_STRING_CHARS - 1, "Late RPG by %d", client->gentity->client->sess.cmd.serverTime - jh_players[player - g_entities].jumpTime);
      JH_util_iprintln(player - g_entities, str);
    }
  }
}

void JH_Callback_BeforeClientThink(client_t *client, usercmd_t *ucmd)
{
  VectorCopy(client->gentity->client->ps.velocity, jh_players[client - svs.clients].oldVelocity);
}

void JH_Callback_AfterClientThink(client_t *client, usercmd_t *ucmd)
{
  gclient_t *gclient = client->gentity->client;
  int clientNum = client - svs.clients;

  if (gclient->sess.sessionState == SESS_STATE_PLAYING)
  {
    bool isOnGround = (gclient->ps.groundEntityNum != 1023);
    bool canBounce = ((gclient->ps.pm_flags & 0x4000) != 0);
    bool bounced = !canBounce && jh_players[clientNum].couldBounce;
    jh_players[clientNum].couldBounce = canBounce;

    //bounce
    if(bounced)
    {
      jh_players[clientNum].bounceTime = gclient->sess.cmd.serverTime;
      if(jh_players[clientNum].RPGTime > gclient->sess.cmd.serverTime - 500)
      {
        char str[MAX_STRING_CHARS];
        snprintf(str, MAX_STRING_CHARS - 1, "Early RPG by %d", gclient->sess.cmd.serverTime - jh_players[clientNum].RPGTime);
        JH_util_iprintln(clientNum, str);
      }
    }

    //land
    if (isOnGround && isOnGround != jh_players[clientNum].wasOnground)
    {
      //landed
      if(jh_players[clientNum].jumpStartOriginSet)
      {
        char str[MAX_STRING_CHARS];
        float offset_x = 30;

        if(jh_players[clientNum].jumpStartOrigin[0] > client->gentity->client->ps.origin[0])
          offset_x = -30;
        float offset_y = 30;
        if(jh_players[clientNum].jumpStartOrigin[1] > client->gentity->client->ps.origin[1])
          offset_y = -30;
        vec3_t diff;
        diff[0] = fabs(client->gentity->client->ps.origin[0] -  jh_players[clientNum].jumpStartOrigin[0] + offset_x);
        diff[1] = fabs(client->gentity->client->ps.origin[1] -  jh_players[clientNum].jumpStartOrigin[1] + offset_y);
        diff[2] = fabs(client->gentity->client->ps.origin[2] -  jh_players[clientNum].jumpStartOrigin[2]);
        float gap = sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
        
        snprintf(str, MAX_STRING_CHARS - 1, "Jumped (%f, %f, %f), gap %f", diff[0], diff[1], diff[2], gap);
        JH_util_iprintln(clientNum, str);
        jh_players[clientNum].jumpStartOriginSet = false;
      }
    }
    jh_players[clientNum].wasOnground = isOnGround;

    //meme mode
    if(jh_players[clientNum].memeMode)
    {
      client->gentity->client->ps.velocity[0] = (2 * client->gentity->client->ps.velocity[0] - jh_players[clientNum].oldVelocity[0]);
      client->gentity->client->ps.velocity[1] = (2 * client->gentity->client->ps.velocity[1] - jh_players[clientNum].oldVelocity[1]);
      if(bounced)
        client->gentity->client->ps.velocity[2] = (1.1 * client->gentity->client->ps.velocity[2] - jh_players[clientNum].oldVelocity[2]);
    }

    //halfbeat
    if(!jh_players[clientNum].halfBeat)
    {
      if(fabs(jh_players[clientNum].oldVelocity[0]) < fabs(client->gentity->client->ps.velocity[0]) && fabs(client->gentity->client->ps.velocity[0]) > 250 && ucmd->forwardmove == 0)
        client->gentity->client->ps.velocity[0] = jh_players[clientNum].oldVelocity[0];
      if(fabs(jh_players[clientNum].oldVelocity[1]) < fabs(client->gentity->client->ps.velocity[1]) && fabs(client->gentity->client->ps.velocity[1]) > 250 && ucmd->forwardmove == 0)
        client->gentity->client->ps.velocity[1] = jh_players[clientNum].oldVelocity[1];
    }

    //autorpg
    if(jh_players[clientNum].autoRPG)
    {
      if(bounced)
      {
        client->gentity->client->ps.velocity[2] += sin(85.0f/360.0f*2.0f*M_PI) * 64;
        client->gentity->client->ps.velocity[0] -= cos(gclient->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
        client->gentity->client->ps.velocity[1] -= cos(gclient->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
      }
    }
  }
  JH_FPS_afterClientThink(client, gclient->sess.cmd.serverTime);
  JH_checkpoints_afterClientThink(client);
}

void JH_Callback_Elevate_Start(struct pmove_t *pm)
{
  int clientNum = pm->ps->clientNum;
  if(!jh_players[clientNum].isElevating)
  {
    jh_players[clientNum].isElevating = true;
    int callback = script_CallBacks_new[SCR_CB_ELEVATE];
    gentity_t *ent = SV_GentityNum(clientNum);
    Scr_AddVector(pm->ps->oldVelocity);
    int threadId = Scr_ExecEntThread(ent, callback, 1);
    Scr_FreeThread(threadId);
  }
}

void JH_Callback_onFrame()
{
  for(int clientNum = 0; clientNum < MAX_CLIENTS; clientNum++)
  {
    client_t *client = &svs.clients[clientNum];
    if(client->state == CS_ACTIVE)
    {
      switch(jh_players[clientNum].nextFrame)
      {
        case NEXTFRAME_LOAD:
        {
          JH_saveload_load(clientNum);
          break;
        }
        default:
        {
          break;
        }
      }
      jh_players[clientNum].nextFrame = NEXTFRAME_NONE;
    }
  }
}

void JH_Callback_Elevate_End(struct pmove_t *pm)
{
  jh_players[pm->ps->clientNum].isElevating = false;
}

void JH_Callback_PlayerConnect(int clientNum)
{
  jh_players[clientNum].isElevating = false;
  jh_players[clientNum].autoRPG = false;
  jh_players[clientNum].memeMode = false;
  jh_players[clientNum].halfBeat = false;
  jh_players[clientNum].fpsFix = false;
  jh_players[clientNum].couldBounce = true;
  jh_players[clientNum].wasOnground = false;
  for(int i = 0; i < FPS_NR_SAMPLES_FPS_AVERAGING; i++)
  {
    jh_players[clientNum].frameTimes[i] = 0;
  }
  jh_players[clientNum].frameNum = 0;
  jh_players[clientNum].prevTime = 0;
  jh_players[clientNum].avgFrameTime = 0;
  jh_players[clientNum].oldVelocity[0] = 0;
  jh_players[clientNum].oldVelocity[1] = 0;
  jh_players[clientNum].oldVelocity[2] = 0;
  JH_saveload_clearSaves(clientNum);
  jh_players[clientNum].checkpoint = NULL;
  jh_players[clientNum].playerState = PLAYERSTATE_PLAYING;
  jh_players[clientNum].RPGTime = 0;
  jh_players[clientNum].bounceTime = 0;
  jh_players[clientNum].jumpTime = 0;
  jh_players[clientNum].jumpStartOriginSet = false;
  jh_players[clientNum].run.runState = RUNSTATE_INITIALIZING;
  jh_players[clientNum].backwardsCount = 0;
  jh_players[clientNum].nextFrame = NEXTFRAME_NONE;
}

void JH_AddFunctions()
{
  Scr_AddFunction("jh_onframe", JH_Callback_onFrame, 0);
  JH_mysql_addFunctions();
  JH_util_addFunctions();
  JH_checkpoints_addFunctions();
}

void JH_AddMethods()
{
  JH_clientCommand_addMethods();
  JH_noclip_addMethods();
  JH_util_addMethods();
  JH_checkpoints_addMethods();
  JH_runs_addMethods();
}