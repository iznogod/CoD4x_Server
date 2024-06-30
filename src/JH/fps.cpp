#include "player.h"
#include "general.h"

extern JH_player *players[];
void JH_fps_FPSFromClientThink(scr_entref_t ent);
bool isHax(int frametime);

void JH_fps_addMethods()
{
    Scr_AddMethod("jh_fps_fpsfromclientthink", JH_fps_FPSFromClientThink, qfalse);
}

void JH_fps_FPSFromClientThink(scr_entref_t ent)
{
    if(players[ent.entnum] != NULL)
        players[ent.entnum]->onFPSClientThink(Scr_GetInt(0));
}

void JH_player::setFPS(JH_fpsInfo _fps)
{
    if((_fps.fpsFlags & 1 << FPSFLAG_HAX) != 0 && (fps.fpsFlags & 1 << FPSFLAG_HAX) == 0)
    {
        //save had hax, current run does not
        warnHax();
    }
    else if((_fps.fpsFlags & 1 << FPSFLAG_MIX) != 0 && (fps.fpsFlags & 1 << FPSFLAG_MIX) == 0)
    {
        //save had mix, current run does not
        warnMix();
    }
    fps.fpsFlags = _fps.fpsFlags;
    if(isHax(fps.frameTimeUserinfo))
    {
        //current fps is hax
        fps.fpsFlags |= 1 << FPSFLAG_HAX;
        fps.fpsFlags |= 1 << FPSFLAG_MIX;
    }
    if(fps.frameTimeUserinfo != _fps.frameTimeUserinfo)
    {
        //different fps from save
        if((fps.fpsFlags & 1 << FPSFLAG_MIX) == 0)
        {
            //no mix flag was set yet
            fps.fpsFlags |= 1 << FPSFLAG_MIX;
            warnMix();
        }
    }
}

int JH_player::getFrameTimeUserinfo()
{
    char *fpsstring = Info_ValueForKey(svs.clients[clientNum].userinfo, "com_maxfps");
    int _fps = atoi(fpsstring);
    if(_fps > 0 && _fps <= 1000)
        return int(1000 / _fps);
    else
        return 0;
}

void JH_player::FPSReset()
{
    int frametime = getFrameTimeUserinfo();
    fps.fpsFlags = 0;
    if(isHax(frametime))
    {
        fps.fpsFlags |= 1 << FPSFLAG_HAX;
        warnHax();
    }
    else
    {
        fps.fpsFlags = 0;
    }
    fps.frameTimeUserinfo = frametime;
}

void JH_player::FPSSpawn()
{
    frameTimeInitialized = false;
    Scr_AddUndefined();
    short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_FPS], 1);
    Scr_FreeThread(ret);

    onFPSUserinfo(getFrameTimeUserinfo());
}

void JH_player::FPSThink(int time)
{
    bool notify = false;
    frameTimes[framenum++] = time - prevTime;
    if(framenum >= FPS_NR_SAMPLES_FPS_AVERAGING)
    {
        framenum = 0;
        if(!frameTimeInitialized)
        {
            notify = true;
            frameTimeInitialized = true;
        }
    }
    if(!frameTimeInitialized)
        return;
    
    float sumFrameTime = 0;
    for(int i = 0; i < FPS_NR_SAMPLES_FPS_AVERAGING; i++)
    {
        sumFrameTime += (float)frameTimes[i];
    }
    int _avgFrameTime = (int)round(sumFrameTime / FPS_NR_SAMPLES_FPS_AVERAGING);
    if(avgFrameTime != _avgFrameTime)
    {
        avgFrameTime = _avgFrameTime;
        notify = true;
    }
    if(notify)
    {
        Scr_AddInt(_avgFrameTime);
        short ret = Scr_ExecEntThread(ent, script_CallBacks_new[SCR_CB_FPS], 1);
        Scr_FreeThread(ret);
    }
}

bool isHax(int frametime)
{
    switch(frametime)
    {
        case 23:
        case 13:
        case 8:
        case 4:
        case 3:
            return false;
        default:
            return true;
    }
}

void JH_player::onFPSUserinfo(int frametime)
{
    if(frametime != fps.frameTimeUserinfo)
    {
        fps.frameTimeUserinfo = frametime;
        onFPS(frametime);
        if(isHax(frametime))
        {
            if((fps.fpsFlags & 1 << FPSFLAG_HAX) == 0)
            {
                warnHax();
                fps.fpsFlags |= 1 << FPSFLAG_MIX;
                fps.fpsFlags |= 1 << FPSFLAG_HAX;
            }
        }
        else
        {
            if((fps.fpsFlags & 1 << FPSFLAG_MIX) == 0)
            {
                warnMix();
                fps.fpsFlags |= 1 << FPSFLAG_MIX;
            }
        }
    }
}

void JH_player::onFPSClientThink(int frametime)
{
    if(frametime != fps.frameTimeUserinfo)
    {
        if(isHax(frametime))
        {
            if((fps.fpsFlags & 1 << FPSFLAG_HAX) == 0)
            {
                warnHax();
                fps.fpsFlags |= 1 << FPSFLAG_MIX;
                fps.fpsFlags |= 1 << FPSFLAG_HAX;
            }
        }
        else
        {
            if((fps.fpsFlags & 1 << FPSFLAG_MIX) == 0)
            {
                warnMix();
                fps.fpsFlags |= 1 << FPSFLAG_MIX;
            }
        }
    }
}

void JH_player::warnHax()
{
    iprintln("^3Hax detected");
}

void JH_player::warnMix()
{
    iprintln("^3Mix detected");
}

void JH_player::onFPS(int frametime)
{
    //todo: fps history etc
    char str[MAX_STRING_CHARS];
    sprintf(str, "FPS changed to %d", getFPSNumber());
    iprintln(str);
}

int JH_player::getFPSNumber()
{
    if(fps.frameTimeUserinfo <= 0 || fps.frameTimeUserinfo > 1000)
        return 0;
    return int(1000 / fps.frameTimeUserinfo);
}

int JH_player::getFPSString(char *str)
{
    if((fps.fpsFlags & 1 << FPSFLAG_HAX) != 0)
        return sprintf(str, "FPS[H]: %d", getFPSNumber());
    else if((fps.fpsFlags & 1 << FPSFLAG_MIX) != 0)
        return sprintf(str, "FPS[M]: %d", getFPSNumber());
    else
        return sprintf(str, "FPS: %d", getFPSNumber());
}