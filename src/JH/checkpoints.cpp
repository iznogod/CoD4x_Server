#include "player.h"
#include "general.h"

int checkpointCount;
JH_checkpoint startCheckpoint;
JH_checkpoint checkpoints[CHECKPOINT_COUNT];

void JH_checkpoints_init();
void JH_checkpoints_add();
void JH_checkpoints_addConnection();
void JH_checkpoints_process();

void JH_checkpoints_addFunctions()
{
    Scr_AddFunction("jh_checkpoints_init", JH_checkpoints_init, qfalse);
    Scr_AddFunction("jh_checkpoints_add", JH_checkpoints_add, qfalse);
    Scr_AddFunction("jh_checkpoints_addconnection", JH_checkpoints_addConnection, qfalse);
    Scr_AddFunction("jh_checkpoints_process", JH_checkpoints_process, qfalse);
}

void JH_checkpoints_init()
{
    checkpointCount = 0;
    startCheckpoint.childCount = 0;
}

void JH_checkpoints_add()
{
    JH_checkpoint *checkpoint = &checkpoints[checkpointCount++];
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

JH_checkpoint *JH_checkpoints_getByID(int id)
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
    JH_checkpoint *parent = JH_checkpoints_getByID(parentID);
    JH_checkpoint *child = JH_checkpoints_getByID(childID);
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
    for(int i = 0; i < checkpointCount; i++)
    {
        if(checkpoints[i].isStart)
        {
            startCheckpoint.children[startCheckpoint.childCount++] = &checkpoints[i];
            if(startCheckpoint.childCount >= CHECKPOINT_CONNECTIONS_MAX)
                startCheckpoint.childCount--;
        }
    }
}

void JH_player::setStartCheckpoint()
{
    setCheckpoint(&startCheckpoint);
}

void JH_player::hideCheckpoints()
{
    Scr_MakeArray();
    short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_DRAWCHECKPOINTS], 1);
    Scr_FreeThread(ret);
}

void JH_player::drawCheckpoints()
{
    if(checkpoint == NULL)
    {
        hideCheckpoints();
    }
    else
    {
        Scr_MakeArray();
        for(int i = 0; i < checkpoint->childCount; i++)
        {
            Scr_AddVector(checkpoint->children[i]->origin);
            Scr_AddArray();
        }
        short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_DRAWCHECKPOINTS], 1);
        Scr_FreeThread(ret);
    }
}

void JH_player::setCheckpoint(JH_checkpoint *_checkpoint)
{
    checkpoint = _checkpoint;
    if(ent->client->sess.sessionState == SESS_STATE_PLAYING && run.state == RUNSTATE_PLAYING)
    {
        drawCheckpoints();
    }
    else
    {
        hideCheckpoints();
    }
}

bool JH_player::isPassingCheckpoint(JH_checkpoint *_checkpoint)
{
    if(!isOnGround() && _checkpoint->onGround)
        return false;
    if(Vec3DistanceSq(ent->client->ps.origin, _checkpoint->origin) > _checkpoint->radiusSquared)
        return false;
    return true;
}

void JH_player::checkpointThink()
{
    if(checkpoint == NULL)
        return;
    if(ent->client->sess.sessionState != SESS_STATE_PLAYING)
        return;
    if(run.state != RUNSTATE_PLAYING)
        return;
    for(int i = 0; i < checkpoint->childCount; i++)
    {
        if(isPassingCheckpoint(checkpoint->children[i]))
        {
            passCheckpoint(checkpoint->children[i]);
            return;
        }
    }
}

void JH_player::passCheckpoint(JH_checkpoint *_checkpoint)
{
    if(_checkpoint->childCount)
    {
        iprintln("^3Checkpoint passed");
        setCheckpoint(_checkpoint);
    }
    else
    {
        if(state == PLAYERSTATE_PLAYING)
        {
            run.state = RUNSTATE_FINISHED;
            pauseRun();
        }
        iprintln("^1Finished");
        setCheckpoint(_checkpoint);
    }
}