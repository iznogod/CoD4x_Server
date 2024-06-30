#include "player.h"
#include "general.h"

void JH_player::hideHUD()
{
    printf("hidehud called\n");
    if(hudState != HUDSTATE_OFF)
    {
        printf("hiding hud\n");
        hudState = HUDSTATE_OFF;
        updateHUD(true);
    }
}

void JH_player::showHUD()
{
    if(hudState != HUDSTATE_ON)
    {
        hudState = HUDSTATE_ON;
        updateHUD(true);
    }
}

void cleanString(char *str)
{
    size_t len = strlen(str);
    for(size_t i = 0; i < len; i++)
    {
        if(str[i] == 146 || str[i] == 34)
            str[i] = 39;
    }
}

void JH_player::makeHUDString(char *str)
{
    int time;
    if(state == PLAYERSTATE_PLAYING)
        time = svs.time - run.startTime;
    else if(state == PLAYERSTATE_PAUSED)
        time = run.stopTime - run.startTime;
    else
        return;
    str += sprintf(str, "Time: ");
    if(time > 60 * 60 * 1000)
        str += sprintf(str, "%d:%02d:%02d", int(time / 60 / 60 / 1000), int((time % (60 * 60 * 1000)) / 60 / 1000), int((time % (60 * 1000)) / 1000));
    else
        str += sprintf(str, "%d:%02d", int(time / 60 / 1000), int((time % (60 * 1000)) / 1000));
    str += sprintf(str, "\nSaves: %d", run.saveCount);
    str += sprintf(str, "\nLoads: %d", run.loadCount);
    str += sprintf(str, "\nRPGs: %d", run.RPGCount);
    str += sprintf(str, "\nDouble RPGs: %d", run.doubleRPGCount);
    str += sprintf(str, "\n");
    str += getFPSString(str);
}

void JH_player::updateHUD(bool force)
{
    if(hudState == HUDSTATE_ON)
    {
        char str[MAX_STRING_CHARS] = "";
        makeHUDString(str);
        cleanString(str);
        if(force || strcmp(str, oldHUDstr))
        {
            strcpy(oldHUDstr, str);
            setClientCvar(SV_CMD_CAN_IGNORE, "jh_hud", str);
        }
    }
    else if(force || oldHUDstr[0] != '\0')
    {
        printf("setting to empty string\n");
        oldHUDstr[0] = '\0';
        setClientCvar(SV_CMD_RELIABLE, "jh_hud", "");
    }
}