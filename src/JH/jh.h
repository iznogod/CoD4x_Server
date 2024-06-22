#ifndef __SCR_JH_H__
#define __SCR_JH_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "../scr_vm.h"
#include "../server.h"
#include "../bg_public.h"
#include "../g_sv_shared.h"
#include "../g_shared.h"
#include "../scr_vm_functions.h"


#include "jh_structs.h"
#include "checkpoints.h"
#include "mysql.h"
#include "clientcommand.h"
#include "saveload.h"
#include "noclip.h"
#include "util.h"
#include "fps.h"
#include "runs.h"

void JH_Callback_Jump(int clientNum, int serverTime, vec3_t origin);
void JH_Callback_RPG(gentity_t *player, gentity_t *rpg);
void JH_Callback_BeforeClientThink(client_t *client, usercmd_t *ucmd);
void JH_Callback_AfterClientThink(client_t *client, usercmd_t *ucmd);
void JH_Callback_Elevate_Start(struct pmove_t *pm);
void JH_Callback_Elevate_End(struct pmove_t *pm);
void JH_Callback_PlayerConnect(int clientNum);
void JH_AddFunctions();
void JH_AddMethods();

#ifdef __cplusplus
};
#endif

#endif