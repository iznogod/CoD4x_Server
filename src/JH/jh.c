#include "jh.h"
#include "mysql.h"
#include "clientcommand.h"
#include "saveload.h"
#include "noclip.h"
#include "util.h"

void JH_Callback_Jump(int clientNum, int serverTime, vec3_t origin)
{
}

void JH_Callback_RPG(gentity_t *player, gentity_t *rpg)
{
}

void JH_Callback_BeforeClientThink(client_t *client, usercmd_t *ucmd)
{
}

void JH_Callback_AfterClientThink(client_t *client, usercmd_t *ucmd)
{
}

void JH_Callback_Elevate_Start(struct pmove_t *pm)
{
}

void JH_Callback_Elevate_End(struct pmove_t *pm)
{
}

void JH_Callback_PlayerConnect(int clientNum)
{
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