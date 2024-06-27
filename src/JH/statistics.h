#ifndef __SCR_STATISTICS_H__
#define __SCR_STATISTICS_H__
#include "jh.h"

void JH_statistics_jump(int clientNum);
void JH_statistics_land(int clientNum);
void JH_statistics_leaveGround(int clientNum);
void JH_statistics_addRPG(int clientNum);
void JH_statistics_spawn(int clientNum);
void JH_statistics_load(int clientNum);

void JH_statistics_bounce(int clientNum);
void JH_statistics_onElevateNormal(int clientNum);
void JH_statistics_onElevateSide(int clientNum);
void JH_statistics_onSave(int clientNum);

#endif