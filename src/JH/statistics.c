#include "statistics.h"

extern JH_PLAYER jh_players[];

void JH_statistics_updateHUD(int clientNum)
{

}

void JH_statistics_onElevateNormal(int clientNum)
{
    jh_players[clientNum].run.runFlags_current |= 1 << RUNFLAG_ELEVATOR_NORMAL;
    if(jh_players[clientNum].run.runFlags_ever & (1 << RUNFLAG_ELEVATOR_NORMAL))
    {
        jh_players[clientNum].run.runFlags_ever |= 1 << RUNFLAG_ELEVATOR_NORMAL;
        JH_statistics_updateHUD(clientNum);
    }
}

void JH_statistics_onElevateSide(int clientNum)
{
    jh_players[clientNum].run.runFlags_current |= 1 << RUNFLAG_ELEVATOR_SIDE;
    if(jh_players[clientNum].run.runFlags_ever & (1 << RUNFLAG_ELEVATOR_SIDE))
    {
        jh_players[clientNum].run.runFlags_ever |= 1 << RUNFLAG_ELEVATOR_SIDE;
        JH_statistics_updateHUD(clientNum);
    }
}

void JH_statistics_onSave(int clientNum)
{
    jh_players[clientNum].run.saveCount++;
    JH_statistics_updateHUD(clientNum);
}

void JH_statistics_addRPG(int clientNum)
{
    jh_players[clientNum].run.RPGCount++;
    jh_players[clientNum].RPGCountJump++;
    if(jh_players[clientNum].RPGCountJump == 2)
    {
        JH_util_iprintln(clientNum, "^1Double RPG detected");
        jh_players[clientNum].run.doubleRPGCount++;
    }
    JH_statistics_updateHUD(clientNum);
}

void JH_statistics_land(int clientNum)
{
    jh_players[clientNum].RPGCountJump = 0;
}

void JH_statistics_leaveGround(int clientNum)
{
    jh_players[clientNum].RPGCountJump = 0;
}

void JH_statistics_jump(int clientNum)
{
    jh_players[clientNum].RPGCountJump = 0;
    jh_players[clientNum].run.jumpCount++;
    JH_statistics_updateHUD(clientNum);
}

void JH_statistics_bounce(int clientNum)
{
    jh_players[clientNum].run.bounceCount++;
    JH_statistics_updateHUD(clientNum);
}

void JH_statistics_spawn(int clientNum)
{
    jh_players[clientNum].RPGCountJump = 0;
}

void JH_statistics_load(int clientNum)
{
    jh_players[clientNum].RPGCountJump = 0;
    jh_players[clientNum].run.loadCount++;
    JH_statistics_updateHUD(clientNum);
}