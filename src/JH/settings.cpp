#include "player.h"

void JH_player::resetSettings()
{
    autoRPG = false;
    memeMode = false;
    halfBeat = false;
    fpsFix = false;
}

void JH_player::setFPSFix(bool val) //todo: store to flags
{
    fpsFix = val;
    if(fpsFix)
        iprintln("^3FPSFix enabled");
    else
        iprintln("^3FPSFix disabled");
}

void JH_player::setAutoRPG(bool val)
{
    autoRPG = val;
    if(autoRPG)
        iprintln("^3AutoRPG enabled");
    else
        iprintln("^3AutoRPG disabled");
}

void JH_player::setMemeMode(bool val)
{
    memeMode = val;
    if(memeMode)
        iprintln("^3Mememode enabled");
    else
        iprintln("^3Mememode disabled");
}

void JH_player::setHalfBeat(bool val)
{
    halfBeat = val;
    if(halfBeat)
        iprintln("^3Halfbeat enabled");
    else
        iprintln("^3Halfbeat disabled");
}