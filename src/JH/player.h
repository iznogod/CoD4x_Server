#ifndef __SCR_PLAYER_H__
#define __SCR_PLAYER_H__

#include "general.h"

typedef enum
{
    HUDSTATE_OFF,
    HUDSTATE_ON,
}JH_hudState;

typedef enum
{
    PLAYERSTATE_NONE,
    PLAYERSTATE_CONNECTING,
    PLAYERSTATE_PLAYING,
    PLAYERSTATE_PAUSED,
    PLAYERSTATE_CINEMATIC
}JH_playerState;

typedef enum
{
    FPSSTATE_NONE,
    FPSSTATE_PURE_43,
    FPSSTATE_PURE_76,
    FPSSTATE_PURE_125,
    FPSSTATE_PURE_250,
    FPSSTATE_PURE_333,
    FPSSTATE_MIX,
    FPSSTATE_HAX
}JH_fpsState;

typedef enum
{
    NEXTFRAME_NONE,
    NEXTFRAME_SPECTATE,
    NEXTFRAME_SPAWN,
    NEXTFRAME_LOAD
}JH_nextFrame;

typedef enum
{
    RUNSTATE_NONE,
    RUNSTATE_INITIALIZING,
    RUNSTATE_PLAYING,
    RUNSTATE_FINISHED
}JH_runState;

typedef struct JH_checkpoint_s
{
    public:
    vec3_t origin;
    int id;
    int radiusSquared;
    bool onGround;
    bool isStart;
    int childCount;
    struct JH_checkpoint_s *children[CHECKPOINT_CONNECTIONS_MAX];
}JH_checkpoint;

class JH_save
{
    public:
    JH_save *prevSave;
    vec3_t origin;
    vec3_t angles;
    int RPGCount;
    int doubleRPGCount;
    int flags;
    JH_fpsState fps;
    JH_checkpoint *checkpoint;
    JH_save(JH_save *prevSave)
    {
        this->prevSave = prevSave;
    }
};

typedef struct
{
    int frameTimes[FPS_NR_SAMPLES_FPS_AVERAGING];
    int framenum;
    int prevTime;
    int avgFrameTime;
    bool initialized;
}JH_fpsInfo;

typedef struct 
{
    public:
    JH_runState state;
    uint32_t flags;
    JH_fpsState fps;
    int runID;
    int startTime;
    int stopTime;
    int saveCount;
    int loadCount;
    int RPGCount;
    int doubleRPGCount;
    int jumpCount;
    int bounceCount;
}JH_run;

class JH_player
{
    public:
    int clientNum;
    JH_run run;
    bool isElevating;
    bool autoRPG;
    bool memeMode;
    bool halfBeat;
    bool fpsFix;
    bool couldBounce;
    bool wasOnGround;
    unsigned int backwardsCount;
    vec3_t oldVelocity;
    bool jumpStartOriginSet;
    vec3_t jumpStartOrigin;
    JH_checkpoint *checkpoint;
    JH_save *save;
    JH_playerState state;
    JH_nextFrame nextFrame;
    int rpgTime;
    int bounceTime;
    int jumpTime;
    int RPGCountJump;
    JH_hudState hudState;
    gentity_t *ent;

    JH_fpsInfo fps;

    int prevTime;

    public:
    virtual ~JH_player(){}
    virtual void spawnPlayer();
    virtual bool canSpawn();
    virtual void onSpawn();
    virtual void onConnect(int _clientNum);
    virtual void onDisconnect();
    virtual void cleanSaves();
    virtual bool canSave();
    virtual bool canLoad();
    virtual void savePosition();
    virtual void loadPosition();
    virtual void setOriginAndAngles(vec3_t origin, vec3_t angles);
    virtual void setCheckpoint(JH_checkpoint *_checkpoint);
    virtual void setStartCheckpoint();
    virtual void drawCheckpoints();
    virtual void hideCheckpoints();
    virtual void passCheckpoint(JH_checkpoint *_checkpoint);
    virtual void checkpointThink();
    virtual bool isPassingCheckpoint(JH_checkpoint *_checkpoint);
    virtual void onSpectate();
    virtual void pauseRun();
    virtual void resumeRun();
    virtual void setFPS(JH_fpsState fps);
    //virtual void setFlags(uint32_t flags);
    virtual void resetRun();
    virtual void intializeRun(int _runID);
    virtual void resetSettings();
    virtual void setFPSFix(bool val);
    virtual void setAutoRPG(bool val);
    virtual void setHalfBeat(bool val);
    virtual void setMemeMode(bool val);
    virtual bool canSpectate();
    virtual void spawnSpectator();
    virtual void onJump(vec3_t origin);
    virtual void onRPGShot(gentity_t *rpg);
    virtual void onRPG();
    virtual void beforeClientThink(usercmd_t *ucmd);
    virtual void afterClientThink(usercmd_t *ucmd);
    virtual void onBounce();
    virtual void onLand();
    virtual void applyAntiHalfbeat(usercmd_t *ucmd);
    virtual void applyAutoRPG(usercmd_t *ucmd);
    virtual void onDoubleRPG();
    virtual void applyMemeMode(bool bounced);
    virtual void onElevate(pmove_t *pm);
    virtual void onFPS(JH_fpsState);
    virtual void FPSThink(int time);
    virtual void FPSReset();

    bool isOnGround()
    {
        return ent->client->ps.groundEntityNum != 1023;
    }
    void iprintln(const char *str)
    {
        SV_GameSendServerCommand(clientNum, 0, va("%c \"%s\"", 'f', str));
    }

    bool canBounce()
    {
        return ((ent->client->ps.pm_flags & 0x4000) != 0);
    }
};

#endif