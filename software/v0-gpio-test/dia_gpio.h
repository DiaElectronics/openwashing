#ifndef dia_gpio_wiringpi
#define dia_gpio_wiringpi

//DO NOT CHANGE PIN COUNT
#define PIN_COUNT 9

#define COIN_TOTAL 6
#define COIN_SWITCH 4

#define NO_ANIMATION 0
#define ONE_BUTTON_ANIMATION 1
#define IDLE_ANIMATION 2
#define BALANCE_ANIMATION 3

#define DEFAULT_LIGHTING_TIME_MS 500

#include <pthread.h>

#include "dia_relayconfig.h"
struct relay_stat {
    long relay_time[PIN_COUNT];
    int relay_switch[PIN_COUNT];
};

class DiaGpio
{
public:
	int AnimationCode;
	int AnimationSubCode;
	relay_stat Stat;

    long curTime;
    int LastPressedKey;
    int InitializedOk;
    int ButtonPin[PIN_COUNT];
    int ButtonLightPin[PIN_COUNT];
    int ButtonLightMoveTo[PIN_COUNT];
    int ButtonLightCurrentPosition[PIN_COUNT];
    int ButtonLightTimeInCurrentPosition[PIN_COUNT];
    int ButtonLightPinStatus[PIN_COUNT];
    int RelayPin[PIN_COUNT];
    int RelayPinStatus[PIN_COUNT];
    long RelayOnTime[PIN_COUNT];

    int NeedWorking;
    int CoinMoney;
    int CoinStatus[COIN_TOTAL];
    int CoinStatus_;
    int CoinLoop;


    int CoinPin;
    int DoorPin;

    int CurrentProgram;
    int AllTurnedOff;
    pthread_t WorkingThread;
    pthread_t LedSwitchingThread;
    DiaRelayConfig Programs[PIN_COUNT];
   DiaGpio();
   ~DiaGpio();
};

void DiaGpio_ButtonAnimation(DiaGpio * gpio, long curTime);
int DiaGpio_GetLastKey(DiaGpio * gpio);
void DiaGpio_WriteRelay(DiaGpio * gpio, int relayNumber, int value);

void DiaGpio_TurnLED(DiaGpio * gpio, int lightNumber, int value);
void * DiaGpio_LedSwitcher(void * gpio);
void DiaGpio_WriteLight(DiaGpio * gpio, int relayNumber, int value);

void DiaGpio_SetProgram(DiaGpio * gpio, int programNumber, int relayNumber,  int onTime, int offTime);
void DiaGpio_StopRelays(DiaGpio * gpio);
int DiaGpio_ReadButton(DiaGpio * gpio, int ButtonNumber);
void DiaGpio_Test(DiaGpio * gpio);
void * DiaGpio_WorkingThread(void * gpio);
inline int DiaGpio_Abs(int from, int to);
void DiaGpio_CheckCoin(DiaGpio * gpio);
//void DiaGpio
#endif
