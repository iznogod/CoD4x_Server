#include "jh.h"
#include "mysql.h"
#include "clientcommand.h"
#include "saveload.h"
#include "noclip.h"
#include "util.h"
#include "fps.h"

JH_PLAYER jh_players[MAX_CLIENTS];

void JH_Callback_Jump(int clientNum, int serverTime, vec3_t origin)
{
  int callback = script_CallBacks_new[SCR_CB_JUMP];

  Scr_AddVector(origin);
  Scr_AddInt(serverTime);
  int threadId = Scr_ExecEntThread(&g_entities[clientNum], callback, 2);
  Scr_FreeThread(threadId);
}

void JH_Callback_RPG(gentity_t *player, gentity_t *rpg)
{
  int callback = script_CallBacks_new[SCR_CB_RPGFIRE];
  Scr_AddInt(player->client->lastServerTime);
  Scr_AddFloat(player->client->ps.viewangles[0]);
  Scr_AddString(BG_GetWeaponDef(rpg->s.weapon)->szInternalName);
  Scr_AddEntity(rpg);
  int threadId = Scr_ExecEntThread(player, callback, 4);
  Scr_FreeThread(threadId);
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
      Scr_AddVector(gclient->ps.velocity);
      Scr_AddInt(gclient->sess.cmd.serverTime);
      short ret = Scr_ExecEntThread(client->gentity, script_CallBacks_new[SCR_CB_BOUNCE], 2);
      Scr_FreeThread(ret);
    }

    //land
    if (isOnGround && isOnGround != jh_players[clientNum].wasOnground)
    {
      Scr_AddVector(gclient->ps.origin);
      Scr_AddInt(ucmd->serverTime);
      short ret = Scr_ExecEntThread(client->gentity, script_CallBacks_new[SCR_CB_LAND], 2);
      Scr_FreeThread(ret);
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
  JH_FPS_AfterClientThink(client, gclient->sess.cmd.serverTime);
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

void JH_Callback_Elevate_End(struct pmove_t *pm)
{
  jh_players[pm->ps->clientNum].isElevating = false;
}

void JH_Callback_PlayerConnect(int clientNum)
{
  jh_players[clientNum].isElevating = false;
  jh_players[clientNum].oldVelocity[0] = 0;
  jh_players[clientNum].oldVelocity[1] = 0;
  jh_players[clientNum].oldVelocity[2] = 0;
  jh_players[clientNum].autoRPG = false;
  jh_players[clientNum].memeMode = false;
  jh_players[clientNum].halfBeat = false;
  jh_players[clientNum].fpsFix = false;
  jh_players[clientNum].couldBounce = true;
  jh_players[clientNum].wasOnground = false;
  for(int i = 0; i < NR_SAMPLES_FPS_AVERAGING; i++)
    jh_players[clientNum].frameTimes[i] = 0;
  jh_players[clientNum].frameNum = 0;
  jh_players[clientNum].prevTime = 0;
  jh_players[clientNum].avgFrameTime = 0;
}

void JH_AddFunctions()
{
  JH_mysql_addFunctions();
  JH_util_addFunctions();
}

void JH_AddMethods()
{
  JH_clientCommand_addMethods();
  JH_saveLoad_addMethods();
  JH_noclip_addMethods();
  JH_util_addMethods();
}