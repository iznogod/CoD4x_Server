#include "clientcommand.h"

extern JH_PLAYER jh_players[];

void JH_clientcommand_ClientCommand(scr_entref_t arg);

void JH_clientCommand_addMethods()
{
    Scr_AddMethod("clientcommand", JH_clientcommand_ClientCommand, 0);
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
    #define MAX_TOKENIZE_STRINGS 32
    char args[MAX_TOKENIZE_STRINGS][MAX_STRING_CHARS];
    int argCount = SV_Cmd_Argc();
    for(int i = 0; i < argCount && i < MAX_TOKENIZE_STRINGS; i++)
    {
        SV_Cmd_ArgvBuffer(i, args[i], sizeof(args[i]));
        for (int j = 0; j < strlen(args[i]); j++)
        {
            if ((unsigned int)args[i][j] < 0x0a)
            {
                args[i][j] = '?';
            }
        }
    }
    printf("cmd is %s\n", args[0]);
    if(!strcasecmp(args[0], "load"))
    {
        int backwardsCount = 0;
        if(argCount > 1)
        {
            backwardsCount = atoi(args[1]);
        }
        if(backwardsCount > 0)
        {
            jh_players[client - svs.clients].backwardsCount += backwardsCount;
        }
        else
        {
            jh_players[client - svs.clients].backwardsCount = 0;
        }
        jh_players[client - svs.clients].nextFrame = NEXTFRAME_LOAD;
    }
    else if(!strcasecmp(args[0], "save"))
    {
        JH_saveload_save(client - svs.clients);
    }
    else
    {
        Scr_MakeArray();
        for(int i = 0; i < argCount && i < MAX_TOKENIZE_STRINGS; i++)
        {
            if (i == 1 && args[i][0] >= 20 && args[i][0] <= 22)
            {
                char *part = strtok(args[i] + 1, " ");
                while (part != NULL)
                {
                    Scr_AddString(part);
                    Scr_AddArray();
                    part = strtok(NULL, " ");
                }
            }
            else
            {
                Scr_AddString(args[i]);
                Scr_AddArray();
            }
        }
        int threadId = Scr_ExecEntThread(&g_entities[client - svs.clients], script_CallBacks_new[SCR_CB_PLAYERCOMMAND], 1);
        Scr_FreeThread(threadId);
    }
}