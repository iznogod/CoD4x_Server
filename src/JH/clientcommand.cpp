#include "player.h"
#include "general.h"

extern JH_player *players[];



void JH_clientcommand_ClientCommand(scr_entref_t entref);
void JH_clientcommand_load(scr_entref_t entref);
void JH_clientcommand_save(scr_entref_t entref);
void JH_clientcommand_spawn(scr_entref_t entref);
void JH_clientcommand_spectate(scr_entref_t entref);
void JH_clientcommand_fpsfix(scr_entref_t entref);
void JH_clientcommand_autorpg(scr_entref_t entref);
void JH_clientcommand_mememode(scr_entref_t entref);
void JH_clientcommand_halfbeat(scr_entref_t entref);

void JH_clientCommand_addMethods()
{
    Scr_AddMethod("clientcommand", JH_clientcommand_ClientCommand, qfalse);
    Scr_AddMethod("jh_spawn", JH_clientcommand_spawn, qfalse);
    Scr_AddMethod("jh_spectate", JH_clientcommand_spectate, qfalse);
    Scr_AddMethod("jh_fpsfix" , JH_clientcommand_fpsfix, qfalse);
    Scr_AddMethod("jh_autorpg", JH_clientcommand_autorpg, qfalse);
    Scr_AddMethod("jh_mememode", JH_clientcommand_mememode, qfalse);
    Scr_AddMethod("jh_halfbeat", JH_clientcommand_halfbeat, qfalse);
}

void JH_clientcommand_halfbeat(scr_entref_t entref)
{
    players[entref.entnum]->setHalfBeat((bool)Scr_GetInt(0));
}

void JH_clientcommand_spawn(scr_entref_t entref)
{
    players[entref.entnum]->nextFrame = NEXTFRAME_SPAWN;
}

void JH_clientcommand_spectate(scr_entref_t entref)
{
    players[entref.entnum]->nextFrame = NEXTFRAME_SPECTATE;
}

void JH_clientcommand_fpsfix(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    players[entref.entnum]->setFPSFix((bool) val);
}

void JH_clientcommand_mememode(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    players[entref.entnum]->setMemeMode((bool) val);
}

void JH_clientcommand_autorpg(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    players[entref.entnum]->setAutoRPG((bool) val);
}

void JH_clientcommand_ClientCommand(scr_entref_t arg)
{
    ClientCommand(arg.entnum);
}

extern "C"
{
void JH_clientcommand_onClientCommand(client_t *client)
{
    int clientNum = client - svs.clients;
    Scr_MakeArray();
    int args = SV_Cmd_Argc();
    for (int i = 0; i < args; i++)
    {
        char tmp[1024];
        SV_Cmd_ArgvBuffer(i, tmp, sizeof(tmp));

        if (i == 1 && tmp[0] >= 20 && tmp[0] <= 22)
        {
            char *part = strtok(tmp + 1, " ");
            while (part != NULL)
            {
                for (size_t j = 0; j < strlen(part); j++)
                {
                    if ((unsigned char)part[j] < 10)
                        part[j] = '?';
                }
                Scr_AddString(part);
                Scr_AddArray();
                part = strtok(NULL, " ");
            }
        }
        else
        {
            for (size_t j = 0; j < strlen(tmp); j++)
            {
                if ((unsigned char)tmp[j] < 10)
                    tmp[j] = '?';
            }
            Scr_AddString(tmp);
            Scr_AddArray();
        }
    }
    int threadId = Scr_ExecEntThread(&g_entities[clientNum], script_CallBacks_new[SCR_CB_PLAYERCOMMAND], 1);
    Scr_FreeThread(threadId);
}
}