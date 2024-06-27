#ifndef __SCR_STRUCTS_H__
#define __SCR_STRUCTS_H__

#define FPS_NR_SAMPLES_FPS_AVERAGING 20
#define CHECKPOINT_CONNECTIONS_MAX 32
#define CHECKPOINT_COUNT 2048

typedef enum
{
    PLAYERSTATE_CONNECTED,
    PLAYERSTATE_PLAYING,
    PLAYERSTATE_PAUSED,
    PLAYERSTATE_SPECTATING,
    PLAYERSTATE_CINEMATIC,
    PLAYERSTATE_DEMO
}JH_PLAYERSTATE;

typedef enum
{
    RUNSTATE_INITIALIZING,
    RUNSTATE_PLAYING,
    RUNSTATE_FINISHED
}JH_RUNSTATE;

typedef enum
{
    RUNFLAG_CHEATING,
    RUNFLAG_HALFBEAT,
    RUNFLAG_ELEVATOR_NORMAL,
    RUNFLAG_ELEVATOR_SIDE,
    RUNFLAG_TAS,
    RUNFLAG_FPSFIX,
    RUNFLAG_AUTORPG,
    RUNFLAG_MEME
}JH_RUNFLAGS;

typedef enum
{
    FPSSTATE_PURE_43,
    FPSSTATE_PURE_76,
    FPSSTATE_PURE_125,
    FPSSTATE_PURE_250,
    FPSSTATE_PURE_333,
    FPSSTATE_MIX,
    FPSSTATE_HAX
}JH_FPSSTATE;

typedef struct
{
    JH_RUNSTATE runState;
    uint32_t runFlags_current;
    uint32_t runFlags_ever;
    JH_FPSSTATE FPSState;
    int runID;
    int startTime;
    int stopTime;
    int saveCount;
    int loadCount;
    int RPGCount;
    int doubleRPGCount;
    int jumpCount;
    int bounceCount;
}JH_RUN;

typedef enum
{
    NEXTFRAME_NONE,
    NEXTFRAME_LOAD,
    NEXTFRAME_SPAWN,
    NEXTFRAME_SPECTATE
}JH_NEXTFRAME;

typedef struct checkpoint_s
{
    vec3_t origin;
    int id;
    int radiusSquared;
    bool onGround;
    bool isStart;
    uint32_t routes;
    int childCount;
    struct checkpoint_s *children[CHECKPOINT_CONNECTIONS_MAX];
}JH_CHECKPOINT;

typedef struct JH_SAVE_s
{
  vec3_t origin;
  vec3_t angles;
  int RPGCount;
  int doubleRPGCount;
  int runFlags;
  JH_FPSSTATE FPSState;
  JH_CHECKPOINT *checkpoint;
  struct JH_SAVE_s *prevSave;
}JH_SAVE;

typedef struct
{
    bool isElevating;
    bool autoRPG;
    bool memeMode;
    bool halfBeat;
    bool fpsFix;
    bool couldBounce;
    bool wasOnground;
    int frameTimes[FPS_NR_SAMPLES_FPS_AVERAGING];
    int frameNum;
    int prevTime;
    int avgFrameTime;
    int backwardsCount;
    vec3_t oldVelocity;
    bool jumpStartOriginSet;
    vec3_t jumpStartOrigin;
    JH_SAVE *save;
    JH_CHECKPOINT *checkpoint;
    JH_PLAYERSTATE playerState;
    JH_RUN run;
    JH_NEXTFRAME nextFrame;
    int RPGTime;
    int bounceTime;
    int jumpTime;
    int RPGCountJump;
}JH_PLAYER;

#endif