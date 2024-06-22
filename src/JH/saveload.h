#ifndef __SCR_SAVELOAD_H__
#define __SCR_SAVELOAD_H__
#include "jh.h"

typedef struct JH_SAVE_s
{
  vec3_t origin;
  vec3_t angles;
  struct JH_SAVE_s *prevSave;
}JH_SAVE;

void JH_saveLoad_addMethods();

#endif