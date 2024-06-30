#ifndef __SCR_PLAYER_H__
#define __SCR_PLAYER_H__

#include "general.h"

typedef enum
{
    HUDSTATE_UNDEFINED,
    HUDSTATE_OFF,
    HUDSTATE_ON,
}JH_hudState;

typedef enum
{
    FPSFLAG_HAX,
    FPSFLAG_MIX
}JH_fpsFlags;

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

typedef struct
{
    uint32_t fpsFlags;
    int frameTimeUserinfo;
}JH_fpsInfo;

class JH_save
{
    public:
    JH_save *prevSave;
    
    vec3_t origin;
    vec3_t angles;
    int RPGCount;
    int doubleRPGCount;
    JH_fpsInfo fps;
    JH_checkpoint *checkpoint;
    JH_save(JH_save *_prevSave)
    {
        prevSave = _prevSave;
    }
};

typedef struct 
{
    public:
    JH_runState state;
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
    char oldHUDstr[MAX_STRING_CHARS];

    int frameTimes[FPS_NR_SAMPLES_FPS_AVERAGING];
    int framenum;
    int avgFrameTime;
    bool frameTimeInitialized;

    JH_fpsInfo fps;

    int prevTime;

    public:
     ~JH_player(){}
    
     void onConnect(int _clientNum);
     void onDisconnect();

     bool canSpawn();
     void spawnPlayer();
     void onSpawn();

     bool canSpectate();
     void spawnSpectator();
     void onSpectate();

     void cleanSaves();
     bool canSave();
     bool canLoad();
     void savePosition();
     void loadPosition();
     void setOriginAndAngles(vec3_t origin, vec3_t angles);

     void setCheckpoint(JH_checkpoint *_checkpoint);
     void setStartCheckpoint();
     void passCheckpoint(JH_checkpoint *_checkpoint);
     void checkpointThink();
     bool isPassingCheckpoint(JH_checkpoint *_checkpoint);

     void drawCheckpoints();
     void hideCheckpoints();
    
     void pauseRun();
     void resumeRun();
     void resetRun();
     void intializeRun(int _runID);

     void resetSettings();
     void setFPSFix(bool val);
     void setAutoRPG(bool val);
     void setHalfBeat(bool val);
     void setMemeMode(bool val);

     void onFPSUserinfo(int frametime);
     void onFPSClientThink(int frametime);

     void onFPS(int frametime);
     void FPSSpawn();
     void FPSThink(int time);
     void FPSReset();
    
     void onJump(vec3_t origin);
     void onRPGShot(gentity_t *rpg);
     void onRPG();
     void onKilled();
     void onBounce();
     void onLand();
     void onDoubleRPG();
     void onElevate(pmove_t *pm);

     void beforeClientThink(usercmd_t *ucmd);
     void afterClientThink(usercmd_t *ucmd);

     void applyAntiHalfbeat(usercmd_t *ucmd);
     void applyAutoRPG(usercmd_t *ucmd);
     void applyMemeMode(bool bounced);

     bool isOnGround();
     void iprintln(const char *str);
     bool canBounce();

     void hideHUD();
     void showHUD();
     void updateHUD(bool force);
     void makeHUDString(char *str);
     int getFPSString(char *str);
     int getFPSNumber();
     void warnHax();
     void warnMix();
     void setFPS(JH_fpsInfo _fps);
     int getFrameTimeUserinfo();

     void setClientCvar(svscmd_type rel, const char *cvar, const char *str);
};

#endif