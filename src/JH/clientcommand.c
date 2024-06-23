#include "clientcommand.h"

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
    int callback = script_CallBacks_new[SCR_CB_PLAYERCOMMAND];
    Scr_MakeArray();
    int args = SV_Cmd_Argc();
    for (int i = 0; i < args; i++)
    {
        char tmp[1024];
        SV_Cmd_ArgvBuffer(i, tmp, sizeof(tmp));
        for (int j = 0; j < strlen(tmp); j++)
        {
            if ((unsigned int)tmp[j] < 0x0a)
            {
                tmp[j] = '?';
            }
        }
        if (i == 1 && tmp[0] >= 20 && tmp[0] <= 22)
        {
            char *part = strtok(tmp + 1, " ");
            while (part != NULL)
            {
                Scr_AddString(part);
                Scr_AddArray();
                part = strtok(NULL, " ");
            }
        }
        else
        {
            Scr_AddString(tmp);
            Scr_AddArray();
        }
    }

    int threadId = Scr_ExecEntThread(&g_entities[client - svs.clients], callback, 1);

    Scr_FreeThread(threadId);
}