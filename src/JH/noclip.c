#include "noclip.h"

void JH_getNoclip(scr_entref_t entref);
void JH_setNoclip(scr_entref_t entref);

void JH_noclip_addMethods()
{
    Scr_AddMethod("setnoclip", JH_setNoclip, qfalse);
	Scr_AddMethod("getnoclip", JH_getNoclip, qfalse);
}

void JH_getNoclip(scr_entref_t entref)
{
  gentity_t *ent = &g_entities[entref.entnum];
  Scr_AddInt(ent->client->noclip);
}

void JH_setNoclip(scr_entref_t entref)
{
  gentity_t *ent = &g_entities[entref.entnum];
  int value = Scr_GetInt(0);
  ent->client->noclip = (qboolean)value;
}