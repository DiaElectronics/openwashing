#include "dia_relayconfig.h"


DiaRelayConfig::DiaRelayConfig()
{
    Price = 0;
    for(int i=0;i<PIN_COUNT_CONFIG;i++)
    {
        RelayNum[i] = -1;
        OnTime[i];
        OffTime[i];
        NextSwitchTime[i];

    }
}
