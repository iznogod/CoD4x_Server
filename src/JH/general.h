#ifndef __SCR_GENERAL_H__
#define __SCR_GENERAL_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "../scr_vm.h"
#include "../server.h"
#include "../bg_public.h"
#include "../g_sv_shared.h"
#include "../g_shared.h"
#include "../scr_vm_functions.h"

#define FPS_NR_SAMPLES_FPS_AVERAGING 20

#define CHECKPOINT_CONNECTIONS_MAX 32
#define CHECKPOINT_COUNT 2048

void JH_checkpoints_addFunctions();
void JH_clientCommand_addMethods();
void JH_util_addFunctions();
void JH_util_addMethods();
void JH_mysql_addFunctions();
void JH_saves_addMethods();
void JH_runs_addMethods();
void JH_fps_addMethods();
void JH_connect_addMethods();

void JH_Callback_PlayerConnect(int clientNum);
void JH_Callback_PlayerKilled(int clientNum);
void JH_Callback_UserInfoChanged(int clientNum);
void JH_Callback_AddFunctions();
void JH_Callback_AddMethods();
void JH_Callback_ClientEndFrame(gentity_t *ent);
void JH_Callback_Jump(int clientNum, vec3_t origin);
void JH_Callback_RPG(gentity_t *player, gentity_t *rpg);
void JH_Callback_BeforeClientThink(client_t *client, usercmd_t *ucmd);
void JH_Callback_AfterClientThink(client_t *client, usercmd_t *ucmd);
void JH_Callback_Elevate_End(struct pmove_t *pm);
void JH_Callback_Elevate_Start(struct pmove_t *pm);
void JH_clientcommand_onClientCommand(client_t *client);

enum svscmd_type
{
	SV_CMD_CAN_IGNORE = 0x0,
	SV_CMD_RELIABLE = 0x1,
};

#ifdef __cplusplus
};
#endif

#endif