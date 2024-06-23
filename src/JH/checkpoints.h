#ifndef __SCR_CHECKPOINTS_H__
#define __SCR_CHECKPOINTS_H__
#include "jh.h"

void JH_checkpoints_addMethods();
void JH_checkpoints_addFunctions();
void JH_checkpoints_afterClientThink(client_t *client);
void JH_checkpoints_setCheckpoint(int clientNum, JH_CHECKPOINT *checkpoint);
void JH_checkpoints_hideCheckpoints(gentity_t *gent);
void JH_checkpoints_drawCheckpoints(gentity_t *gent);

#endif