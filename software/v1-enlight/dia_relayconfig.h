#ifndef dia_relayconfig_wash
#define dia_relayconfig_wash
#define PIN_COUNT_CONFIG 9


class DiaRelayConfig
{
public:
    int RelayNum[PIN_COUNT_CONFIG];
    long OnTime[PIN_COUNT_CONFIG];
    long OffTime[PIN_COUNT_CONFIG];
    long NextSwitchTime[PIN_COUNT_CONFIG];
    DiaRelayConfig() {
        for(int i=0;i<PIN_COUNT_CONFIG;i++) {
            RelayNum[i] = -1;
            OnTime[i]=0;
            OffTime[i]=0;
            NextSwitchTime[i]=0;
        }
    }
    int InitRelay(int id, int ontime, int offtime) {
        //printf("RELAY_CONFIG:id:%d,on:%d,off:%d\n",id, ontime, offtime);
        if (id<=0 && id>=PIN_COUNT_CONFIG) {
            printf("err, relay id can't be 0 or less or more than pin count [%d] \n", id);
            return 1;
        }
        if(ontime<0 || offtime<0) {
            ontime = 1000;
            offtime = 0;
            printf("error: check relays conf\n");
        }
        this->RelayNum[id] = id;
        this->OnTime[id] =  ontime;
        this->OffTime[id] = offtime;
        //printf("AFTER_ASSIGNMENT:id:%d,on:%ld,off:%ld\n",this->RelayNum[id], 
        //    this->OnTime[id], 
        //    this->OffTime[id]);
        return 0;
    }
};

#endif
