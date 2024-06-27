#include "clientcommand.h"

extern JH_PLAYER jh_players[];

void JH_clientcommand_ClientCommand(scr_entref_t entref);
void JH_clientcommand_load(scr_entref_t entref);
void JH_clientcommand_save(scr_entref_t entref);
void JH_clientcommand_spawn(scr_entref_t entref);
void JH_clientcommand_spectate(scr_entref_t entref);
void JH_clientcommand_fpsfix(scr_entref_t entref);
void JH_clientcommand_mememode(scr_entref_t entref);
void JH_clientcommand_autorpg(scr_entref_t entref);

void JH_clientCommand_addMethods()
{
    Scr_AddMethod("clientcommand", JH_clientcommand_ClientCommand, 0);
    Scr_AddMethod("jh_load", JH_clientcommand_load, 0);
    Scr_AddMethod("jh_save", JH_clientcommand_save, 0);
    Scr_AddMethod("jh_spawn", JH_clientcommand_spawn, 0);
    Scr_AddMethod("jh_spectate", JH_clientcommand_spectate, 0);
    Scr_AddMethod("jh_fpsfix" , JH_clientcommand_fpsfix, 0);
    Scr_AddMethod("jh_mememode", JH_clientcommand_mememode, 0);
    Scr_AddMethod("jh_autorpg", JH_clientcommand_autorpg, 0);
}

void JH_clientcommand_load(scr_entref_t entref)
{
    int backwardsCount = Scr_GetInt(0);
    jh_players[entref.entnum].nextFrame = NEXTFRAME_LOAD;
    jh_players[entref.entnum].backwardsCount += backwardsCount;
}

void JH_clientcommand_save(scr_entref_t entref)
{
    JH_saveload_save(entref.entnum);
}

void JH_clientcommand_spawn(scr_entref_t entref)
{
    jh_players[entref.entnum].nextFrame = NEXTFRAME_SPAWN;
}

void JH_clientcommand_spectate(scr_entref_t entref)
{
    jh_players[entref.entnum].nextFrame = NEXTFRAME_SPECTATE;
}

void JH_clientcommand_fpsfix(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    jh_players[entref.entnum].fpsFix = (bool) val;
    if(jh_players[entref.entnum].fpsFix)
    {
        JH_util_iprintln(entref.entnum, "^2FPSFix enabled");
    }
    else
    {
        JH_util_iprintln(entref.entnum, "^1FPSFix disabled");
    }
}

void JH_clientcommand_mememode(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    jh_players[entref.entnum].memeMode = (bool) val;
    if(jh_players[entref.entnum].memeMode)
    {
        JH_util_iprintln(entref.entnum, "^2Meme mode enabled");
    }
    else
    {
        JH_util_iprintln(entref.entnum, "^1Meme mode disabled");
    }
}

void JH_clientcommand_autorpg(scr_entref_t entref)
{
    int val = Scr_GetInt(0);
    jh_players[entref.entnum].autoRPG = (bool) val;
    if(jh_players[entref.entnum].autoRPG)
    {
        JH_util_iprintln(entref.entnum, "^2AutoRPG mode enabled");
    }
    else
    {
        JH_util_iprintln(entref.entnum, "^1AutoRPG mode disabled");
    }
}

void JH_clientcommand_ClientCommand(scr_entref_t arg)
{
    mvabuf;

    if (arg.classnum)
    {
        Scr_ObjectError("Not an entity");
    }
    else
    {
        int entityNum = arg.entnum;
        gentity_t *gentity = &g_entities[entityNum];

        if (!gentity->client)
        {
            Scr_ObjectError(va("Entity: %i is not a player", entityNum));
        }
        else
        {
            if (Scr_GetNumParam() != 0)
            {
                Scr_Error("Usage: self ClientCommand()\n");
            }
            else
            {
                ClientCommand(entityNum);
            }
        }
    }
}

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
                for (int j = 0; j < strlen(part); j++)
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
            for (int j = 0; j < strlen(tmp); j++)
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