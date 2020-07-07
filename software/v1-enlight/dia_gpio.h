#ifndef dia_gpio_wiringpi
#define dia_gpio_wiringpi

//DO NOT CHANGE PIN COUNT
#define PIN_COUNT 18

#define COIN_TOTAL 6
#define COIN_SWITCH 4
#define COIN_PIN 29
#define BANKNOTE_PIN 28

#define NO_ANIMATION 0
#define ONE_BUTTON_ANIMATION 1
#define IDLE_ANIMATION 2
#define BALANCE_ANIMATION 3
#define FREEZE_ANIMATION 4

#define DEFAULT_LIGHTING_TIME_MS 500
#define MAX_PROGRAMS_COUNT 100

#include <pthread.h>
#include <map>

#include "dia_relayconfig.h"
#include "dia_storage_interface.h"

struct relay_stat {
    long relay_time[PIN_COUNT];
    int relay_switch[PIN_COUNT];
};

class PulseHandler {
public:
  int PinNumber;
  int Money;
  int Status[COIN_TOTAL];
  int Status_;
  int Loop;
private:
  int calcONPins() {
    int res = 0;
    for(int i=0;i < COIN_TOTAL;i++) {
      if(Status[i]) res++;
    }
    return res;
  }

public:
  PulseHandler(int pinNumber) {
    Money = 0;
    PinNumber=pinNumber;
    pinMode(PinNumber, INPUT);
    for(int i=0;i<COIN_TOTAL;i++) {
      Status[i] = digitalRead(PinNumber);
      Status_ = digitalRead(PinNumber);
    }
    Loop = 0;
  }

  void Tick() {
    int curState = digitalRead(PinNumber);
    Status[Loop] = curState;

    Loop++;
    if(Loop>=COIN_TOTAL) Loop=0;

    int curSwitchedOnPins = calcONPins();
    
    if(Status_) {
      if(curSwitchedOnPins<(COIN_TOTAL-COIN_SWITCH)) Status_ = 0;
    } else {
      if(curSwitchedOnPins>COIN_SWITCH) {
          Money++;
          Status_ = 1;
      }
    }
  }
};

class DiaGpio {
public:
    int MaxButtons;
    int MaxRelays;
    
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

    int ButtonStatus[PIN_COUNT];
    long ButtonLastStatusChangeTime[PIN_COUNT];

    int RelayPin[PIN_COUNT];
    int RelayPinStatus[PIN_COUNT];
    long RelayOnTime[PIN_COUNT];


    int NeedWorking;

    PulseHandler * CoinsHandler;
    PulseHandler * BanknotesHandler;

    int CurrentProgram;
    int AllTurnedOff;

    pthread_t WorkingThread;
    pthread_t LedSwitchingThread;


    DiaRelayConfig Programs[MAX_PROGRAMS_COUNT];
    std::map<std::string, int> _ProgramMapping;
    DiaGpio(int maxButtons, int maxRelays, storage_interface_t * storage);
    ~DiaGpio();
private:
    storage_interface_t * _Storage;
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
//void DiaGpio
#endif
