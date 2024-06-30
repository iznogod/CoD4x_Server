#include "player.h"
#include "general.h"

void JH_player::FPSReset()
{
    fps.initialized = false;
}

void JH_player::FPSThink(int time)
{
    fps.frameTimes[fps.framenum++] = time - prevTime;
    if(fps.framenum >= FPS_NR_SAMPLES_FPS_AVERAGING)
    {
        fps.framenum = 0;
        fps.initialized = true;
    }
    if(!fps.initialized)
        return;
    
    float sumFrameTime = 0;
    for(int i = 0; i < FPS_NR_SAMPLES_FPS_AVERAGING; i++)
    {
        sumFrameTime += (float)fps.frameTimes[i];
    }
    int _avgFrameTime = (int)round(sumFrameTime / FPS_NR_SAMPLES_FPS_AVERAGING);
    if(fps.avgFrameTime != _avgFrameTime)
    {
        fps.avgFrameTime = _avgFrameTime;
        Scr_AddInt(_avgFrameTime);
        short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_FPS], 1);
        Scr_FreeThread(ret);
    }
}

void JH_player::onFPS(JH_fpsState _fps)
{
    if(_fps == FPSSTATE_HAX)
        setFPS(FPSSTATE_HAX);
    else if(_fps == FPSSTATE_MIX && run.fps != FPSSTATE_HAX)
        setFPS(FPSSTATE_MIX);
    else if(_fps != run.fps && run.fps != FPSSTATE_NONE)
        setFPS(FPSSTATE_MIX);
    else
        setFPS(_fps);
}

void JH_player::setFPS(JH_fpsState _fps)
{
    if(_fps != run.fps)
    {
        run.fps = _fps;
        if(_fps == FPSSTATE_NONE)
            return;
        const char *fpsstring = "";
        switch(_fps)
        {

            case FPSSTATE_HAX:
                fpsstring =  "HAX";
                break;
            case FPSSTATE_MIX:
                fpsstring = "MIX";
                break;
            case FPSSTATE_PURE_43:
                fpsstring = "43 fps";
                break;
            case FPSSTATE_PURE_76:
                fpsstring = "76 fps";
                break;
            case FPSSTATE_PURE_125:
                fpsstring = "125 fps";
                break;
            case FPSSTATE_PURE_250:
                fpsstring = "250 fps";
                break;
            case FPSSTATE_PURE_333:
                fpsstring = "333 fps";
                break;
            case FPSSTATE_NONE:
                break;
        }
        char str[MAX_STRING_CHARS];
        sprintf(str, "^3FPS changed to %s", fpsstring);
        iprintln("^3FPS changed to ");
    }
}