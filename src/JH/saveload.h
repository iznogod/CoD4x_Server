#ifndef __SCR_SAVELOAD_H__
#define __SCR_SAVELOAD_H__
#include "jh.h"

void JH_saveLoad_addMethods();
void JH_saveload_clearSaves(int entnum);
void JH_saveload_save(int entnum);
void JH_saveload_load(int entnum);

#endif