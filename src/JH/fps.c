#include "fps.h"


extern JH_PLAYER jh_players[];

void JH_FPS_AfterClientThink(client_t *client, int time)
{
    int clientNum = client - svs.clients;
    jh_players[clientNum].frameTimes[jh_players[clientNum].frameNum] = time - jh_players[clientNum].prevTime;
    jh_players[clientNum].prevTime = time;

    // There are x sample slots, if all are used we restart at begin
    if (++jh_players[clientNum].frameNum >= NR_SAMPLES_FPS_AVERAGING)
    {
        jh_players[clientNum].frameNum = 0;
    }

    // Sum frame times so we can use it to calculate the average
    float sumFrameTime = 0;
    for (int i = 0; i < NR_SAMPLES_FPS_AVERAGING; i++)
    {
        sumFrameTime += (float)jh_players[clientNum].frameTimes[i];
    }
    // Check if client frame time is different from what we previously reported
    int avgFrameTime = (int)round(sumFrameTime / NR_SAMPLES_FPS_AVERAGING);
    if (jh_players[clientNum].avgFrameTime != avgFrameTime)
    {
        // Client FPS changed, report this to GSC via callback
        jh_players[clientNum].avgFrameTime = avgFrameTime;
        Scr_AddInt(avgFrameTime);
        short ret = Scr_ExecEntThread(client->gentity, script_CallBacks_new[SCR_CB_FPS], 1);
        Scr_FreeThread(ret);
    }
}