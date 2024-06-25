#include "jh.h"
JH_PLAYER jh_players[MAX_CLIENTS] = {0};

void JH_Callback_ClientEndFrame(gentity_t *ent)
{
  int clientNum = ent - g_entities;
  client_t *client = &svs.clients[clientNum];
  
  if(client->state == CS_ACTIVE)
  {
    JH_NEXTFRAME action = jh_players[clientNum].nextFrame;
    jh_players[clientNum].nextFrame = NEXTFRAME_NONE;
    switch(action)
    {
      case NEXTFRAME_LOAD:
      {
        JH_saveload_load(clientNum);
        break;
      }
      case NEXTFRAME_SPAWN:
      {
        int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_SPAWN], 0);
        Scr_FreeThread(threadId);
        JH_checkpoints_drawCheckpoints(ent);
        jh_players[clientNum].playerState = PLAYERSTATE_PLAYING;
        break;
      }
      case NEXTFRAME_SPECTATE:
      {
        int threadId = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_SPECTATE], 0);
        Scr_FreeThread(threadId);
        JH_checkpoints_hideCheckpoints(ent);
        jh_players[clientNum].playerState = PLAYERSTATE_SPECTATING;
        break;
      }
      default:
      {
        break;
      }
    }
  }
  ClientEndFrame(ent);
}

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
  char str[MAX_STRING_CHARS];
  snprintf(str, MAX_STRING_CHARS - 1, "RPG angle: %.2f", client->gentity->client->ps.viewangles[0]);
  JH_util_iprintln(player - g_entities, str);
  if(jh_players[player - g_entities].bounceTime > jh_players[player - g_entities].jumpTime)
  {
    if(jh_players[player - g_entities].bounceTime > client->gentity->client->sess.cmd.serverTime - 500)
    {
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
  //fpsfix
  if(jh_players[client - svs.clients].fpsFix)
  {
    char *fpsstring = Info_ValueForKey(client->userinfo, "com_maxfps");
    int fps = atoi(fpsstring);
    if(fps > 0 && fps <= 1000)
    {
      int actualFrameTime = ucmd->serverTime - jh_players[client - svs.clients].prevTime;
      int wantedFrameTime = 1000 / fps;
      if(actualFrameTime != wantedFrameTime && abs(actualFrameTime - wantedFrameTime) < 10 && abs(actualFrameTime - wantedFrameTime) <= actualFrameTime / 2)
      {
        // adjust frametime
        client->lastUsercmd.serverTime = client->lastUsercmd.serverTime - actualFrameTime + wantedFrameTime;
        ucmd->serverTime = ucmd->serverTime - actualFrameTime + wantedFrameTime;
      }
    }
  }
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

        if(jh_players[clientNum].jumpStartOrigin[0] > gclient->ps.origin[0])
          offset_x = -30;
        float offset_y = 30;
        if(jh_players[clientNum].jumpStartOrigin[1] > gclient->ps.origin[1])
          offset_y = -30;
        vec3_t diff;
        diff[0] = fabs(gclient->ps.origin[0] -  jh_players[clientNum].jumpStartOrigin[0] + offset_x);
        diff[1] = fabs(gclient->ps.origin[1] -  jh_players[clientNum].jumpStartOrigin[1] + offset_y);
        diff[2] = fabs(gclient->ps.origin[2] -  jh_players[clientNum].jumpStartOrigin[2]);
        float gap = sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
        
        snprintf(str, MAX_STRING_CHARS - 1, "Jumped (%.2f, %.2f, %.2f), gap %.2f", diff[0], diff[1], diff[2], gap);
        JH_util_iprintln(clientNum, str);
        jh_players[clientNum].jumpStartOriginSet = false;
      }
    }
    jh_players[clientNum].wasOnground = isOnGround;

    //meme mode
    if(jh_players[clientNum].memeMode)
    {
      gclient->ps.velocity[0] = (2 * gclient->ps.velocity[0] - jh_players[clientNum].oldVelocity[0]);
      gclient->ps.velocity[1] = (2 * gclient->ps.velocity[1] - jh_players[clientNum].oldVelocity[1]);
      if(bounced)
        gclient->ps.velocity[2] = (1.1 * gclient->ps.velocity[2] - jh_players[clientNum].oldVelocity[2]);
    }

    //halfbeat
    if(!jh_players[clientNum].halfBeat && !isOnGround)
    {
      int frametime = ucmd->serverTime - jh_players[clientNum].prevTime;
      int accel = round((float)(frametime * g_gravity->current.floatval) * 0.001);
      if(jh_players[clientNum].oldVelocity[2] - gclient->ps.velocity[2] == accel)
      {
        if(fabs(jh_players[clientNum].oldVelocity[0]) < fabs(gclient->ps.velocity[0]) && fabs(gclient->ps.velocity[0]) > 250 && ucmd->forwardmove == 0)
          gclient->ps.velocity[0] = jh_players[clientNum].oldVelocity[0];
        if(fabs(jh_players[clientNum].oldVelocity[1]) < fabs(gclient->ps.velocity[1]) && fabs(gclient->ps.velocity[1]) > 250 && ucmd->forwardmove == 0)
          gclient->ps.velocity[1] = jh_players[clientNum].oldVelocity[1];
      }
      else
      {
        //player is sliding
      }
    }

    //autorpg
    if(jh_players[clientNum].autoRPG)
    {
      if(bounced)
      {
        gclient->ps.velocity[2] += sin(85.0f/360.0f*2.0f*M_PI) * 64;
        gclient->ps.velocity[0] -= cos(gclient->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
        gclient->ps.velocity[1] -= cos(gclient->ps.viewangles[1] / 360.0f * 2.0f * M_PI);
      }
    }
  }
  JH_FPS_afterClientThink(client, gclient->sess.cmd.serverTime);
  JH_checkpoints_afterClientThink(client);
  jh_players[clientNum].prevTime = gclient->sess.cmd.serverTime;
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
  jh_players[clientNum].playerState = PLAYERSTATE_CONNECTED;
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