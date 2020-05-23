#ifndef dia_relayconfig_wash
#define dia_relayconfig_wash
#define PIN_COUNT_CONFIG 9


class DiaRelayConfig
{
public:
    int RelayNum[PIN_COUNT_CONFIG];
    int OnTime[PIN_COUNT_CONFIG];
    int OffTime[PIN_COUNT_CONFIG];
    int NextSwitchTime[PIN_COUNT_CONFIG];
    int Price;
    DiaRelayConfig();
};

#endif
