#include "checkpoints.h"

int checkpointCount;
extern JH_PLAYER jh_players[];
JH_CHECKPOINT checkpoints[CHECKPOINT_COUNT];
JH_CHECKPOINT startCheckpoint;

void JH_checkpoints_drawCheckpoints(gentity_t *gent);
void JH_checkpoints_init();
void JH_checkpoints_add();
void JH_checkpoints_addConnection();
void JH_checkpoints_process();
void JH_checkpoints_notifyPass(gentity_t *gent, JH_CHECKPOINT *checkpoint);
void JH_checkpoints_notifyFinish(gentity_t *gent, JH_CHECKPOINT *checkpoint);

void JH_checkpoints_addMethods()
{
}

void JH_checkpoints_addFunctions()
{
    Scr_AddFunction("jh_checkpoints_init", JH_checkpoints_init, 0);
    Scr_AddFunction("jh_checkpoints_add", JH_checkpoints_add, 0);
    Scr_AddFunction("jh_checkpoints_addconnection", JH_checkpoints_addConnection, 0);
    Scr_AddFunction("jh_checkpoints_process", JH_checkpoints_process, 0);
}

void JH_checkpoints_init()
{
    printf("cp init\n");
    checkpointCount = 0;
    startCheckpoint.childCount = 0;
}

void JH_checkpoints_add()
{
    printf("cp add\n");
    JH_CHECKPOINT *checkpoint = &checkpoints[checkpointCount++];
    if(checkpointCount >= CHECKPOINT_COUNT)
        checkpointCount--;
    
    checkpoint->id = Scr_GetInt(0);
    Scr_GetVector(1, checkpoint->origin);
    int radius = Scr_GetInt(2);
    checkpoint->radiusSquared = radius * radius;
    checkpoint->onGround = (bool) Scr_GetInt(3);
    checkpoint->isStart = true;
    checkpoint->childCount = 0;
}

JH_CHECKPOINT *JH_checkpoints_getByID(int id)
{
    for(int i = 0; i < checkpointCount; i++)
    {
        if(checkpoints[i].id == id)
            return &checkpoints[i];
    }
    return NULL;
}

void JH_checkpoints_addConnection()
{
    printf("cp add conn\n");
    int parentID = Scr_GetInt(0);
    int childID = Scr_GetInt(1);
    JH_CHECKPOINT *parent = JH_checkpoints_getByID(parentID);
    JH_CHECKPOINT *child = JH_checkpoints_getByID(childID);
    if(parent != NULL && child != NULL)
    {
        printf("Connection found, connecting\n");
        parent->children[parent->childCount++] = child;
        if(parent->childCount >= CHECKPOINT_CONNECTIONS_MAX)
            parent->childCount--;
        child->isStart = false;
    }
}

void JH_checkpoints_process()
{
    printf("cp process\n");
    for(int i = 0; i < checkpointCount; i++)
    {
        if(checkpoints[i].isStart)
        {
            printf("Adding start checkpoint\n");
            startCheckpoint.children[startCheckpoint.childCount++] = &checkpoints[i];
            if(startCheckpoint.childCount >= CHECKPOINT_CONNECTIONS_MAX)
                startCheckpoint.childCount--;
        }
    }
}

void JH_checkpoints_setStartCheckpoint(int clientNum)
{
    printf("cp start\n");
    jh_players[clientNum].checkpoint = &startCheckpoint;
    JH_checkpoints_drawCheckpoints(&g_entities[clientNum]);
}

void JH_checkpoints_afterClientThink(client_t *client)
{
    gclient_t *gclient = client->gentity->client;
    int clientNum = client - svs.clients;
    if(jh_players[clientNum].checkpoint == NULL)
        return;
    for(int i = 0; i < jh_players[clientNum].checkpoint->childCount; i++)
    {
        if((!jh_players[clientNum].checkpoint->children[i]->onGround || gclient->ps.groundEntityNum != 1023) && Vec3DistanceSq(gclient->ps.origin, jh_players[clientNum].checkpoint->children[i]->origin) < jh_players[clientNum].checkpoint->children[i]->radiusSquared)
        {
            //player passed checkpoint
            JH_CHECKPOINT *checkpoint = jh_players[clientNum].checkpoint->children[i];
            jh_players[clientNum].checkpoint = jh_players[clientNum].checkpoint->children[i];
            if(jh_players[clientNum].checkpoint->childCount == 0)
            {
                JH_checkpoints_notifyFinish(client->gentity, checkpoint);
            }
            else
            {
                JH_checkpoints_notifyPass(client->gentity, checkpoint);
            }
            JH_checkpoints_drawCheckpoints(client->gentity);
            break;
        }
    }
}

void JH_checkpoints_setCheckpoint(int clientNum, JH_CHECKPOINT *checkpoint)
{
    if(jh_players[clientNum].checkpoint != checkpoint)
    {
        jh_players[clientNum].checkpoint = checkpoint;
        JH_checkpoints_drawCheckpoints(&g_entities[clientNum]);
    }
}

void JH_checkpoints_notifyPass(gentity_t *gent, JH_CHECKPOINT *checkpoint)
{
    printf("passed checkpoint\n");
}

void JH_checkpoints_notifyFinish(gentity_t *gent, JH_CHECKPOINT *checkpoint)
{
    printf("finished\n");
}

void JH_checkpoints_hideCheckpoints(gentity_t *gent)
{
    Scr_MakeArray();
    short ret = Scr_ExecEntThread(gent, script_CallBacks_new[SCR_CB_DRAWCHECKPOINTS], 1);
    Scr_FreeThread(ret);
}

void JH_checkpoints_drawCheckpoints(gentity_t *gent)
{
    Scr_MakeArray();
    for(int i = 0; i < jh_players[gent - g_entities].checkpoint->childCount; i++)
    {
        Scr_AddVector(jh_players[gent - g_entities].checkpoint->children[i]->origin);
        Scr_AddArray();
    }
    short ret = Scr_ExecEntThread(gent, script_CallBacks_new[SCR_CB_DRAWCHECKPOINTS], 1);
    Scr_FreeThread(ret);
}