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

#define NR_SAMPLES_FPS_AVERAGING 20

typedef struct
{
    bool isElevating;
    bool autoRPG;
    bool memeMode;
    bool halfBeat;
    bool fpsFix;
    bool couldBounce;
    bool wasOnground;
    int frameTimes[NR_SAMPLES_FPS_AVERAGING];
    int frameNum;
    int prevTime;
    int avgFrameTime;
    vec3_t oldVelocity;
}JH_PLAYER;

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