#include <stdio.h>
#include <wiringPi.h>

#include "dia_gpio.h"
#include "pthread.h"
#include <assert.h>

DiaGpio::~DiaGpio() {
  delete CoinsHandler;
  delete BanknotesHandler;
}

int DiaGpio_GetLastKey(DiaGpio * gpio) {
    if(gpio->LastPressedKey>=0) {
        int res = gpio->LastPressedKey;
        gpio->LastPressedKey = 0;
        return res;
    }
    return 0;
}

void DiaGpio_TurnLED(DiaGpio * gpio, int lightNumber, int value) {
    if(value<=0) {
        value = 0;
    } else {
        value = 1;
    }
    if(gpio->InitializedOk) {
        if(lightNumber>=0 && lightNumber<=gpio->MaxButtons && gpio->ButtonLightPin[lightNumber]>=0) {
            digitalWrite(gpio->ButtonLightPin[lightNumber], value);
            gpio->ButtonLightPinStatus[lightNumber] = value;
            gpio->ButtonLightTimeInCurrentPosition[lightNumber] = 0;
        }
    }
}

void DiaGpio_WriteLight(DiaGpio * gpio, int lightNumber, int value) {
    if(lightNumber>=0 && lightNumber<PIN_COUNT) {
        gpio->ButtonLightMoveTo[lightNumber] = value;
    }
}

void DiaGpio_WriteRelay(DiaGpio * gpio, int relayNumber, int value) {
    if(value<=0) {
        value = 0;
    } else {
        value = 1;
    }
    if(gpio->InitializedOk) {
        if(relayNumber>=0 && relayNumber<=gpio->MaxRelays && gpio->RelayPin[relayNumber]>=0) {
            if (value == 1) {
                gpio->RelayOnTime[relayNumber] = gpio->curTime;
            } else {
                if (gpio->RelayOnTime[relayNumber]>0) {
                    long time_elapsed = gpio->curTime - gpio->RelayOnTime[relayNumber];
                    gpio->Stat.relay_time[relayNumber] = gpio->Stat.relay_time[relayNumber] + time_elapsed;
                    gpio->Stat.relay_switch[relayNumber] = gpio->Stat.relay_switch[relayNumber] + 1;
                    gpio->RelayOnTime[relayNumber] = 0;
                }
            }
            //printf("R%d:[%d]\n", relayNumber, value);
            if(gpio->RelayPinStatus[relayNumber]!=value) {
                digitalWrite(gpio->RelayPin[relayNumber], value);
                gpio->RelayPinStatus[relayNumber] = value;
            }
        }
    }
}

void cleanPins(int * arr, int N) {
  assert(arr);
  for (int i=0; i<N; i++) {
      arr[i] = -1;
  }
}

DiaGpio::DiaGpio(int maxButtons, int maxRelays, storage_interface_t * storage) {
    InitializedOk = 0;
    MaxButtons = maxButtons;
    MaxRelays = maxRelays;
    CurrentProgram = -1;
    AllTurnedOff = 0;
    if(maxButtons>=PIN_COUNT) {
        printf("ERROR: buttons # is too big\n");
        InitializedOk = 0;
        return;
    }
    if(maxRelays>=PIN_COUNT) {
        printf("ERROR: relays # is too big\n");
        InitializedOk = 0;
        return;
    }
    AnimationCode = 0;
    AnimationSubCode = 0;
    LastPressedKey = -1;
    InitializedOk = 0;
    NeedWorking = 1;

    if (wiringPiSetup () == -1) return;

    CoinsHandler = new PulseHandler(COIN_PIN);
    BanknotesHandler = new PulseHandler(BANKNOTE_PIN);
    AdditionalHandler = 0;

    cleanPins(ButtonPin, PIN_COUNT);
    ButtonPin[1] = 13;
    ButtonPin[2] = 14;
    ButtonPin[3] = 21;
    ButtonPin[4] = 22;
    ButtonPin[5] = 23;
    ButtonPin[6] = 24;
    ButtonPin[7] = 25;

    cleanPins(ButtonLightPin, PIN_COUNT);
    ButtonLightPin[1] = 8;
    ButtonLightPin[2] = 9;
    ButtonLightPin[3] = 7;
    ButtonLightPin[4] = 0;
    ButtonLightPin[5] = 2;
    ButtonLightPin[6] = 3;
    ButtonLightPin[7] = 12;

    cleanPins(RelayPin, PIN_COUNT);
    RelayPin[1] = 15; // HighPreasurePump
    RelayPin[2] = 16; // Not USED
    RelayPin[3] = 1; // Soap
    RelayPin[4] = 4; // Wax
    RelayPin[5] = 5; // Pump Station
    RelayPin[6] = 6; // Light
 
    // these pins overwrite button lights
    RelayPin[11] = 8; 
    RelayPin[12] = 9; 
    RelayPin[13] = 7; 
    RelayPin[14] = 0; 
    RelayPin[15] = 2; 
    RelayPin[16] = 3;
    RelayPin[17] = 12;

    for(int i=0;i<PIN_COUNT;i++) {
        if(RelayPin[i] >= 0) {
            pinMode(RelayPin[i], OUTPUT);
            digitalWrite(RelayPin[i],0);
        }
        if(ButtonPin[i]>=0) {
            pinMode(ButtonPin[i], INPUT);
        }
        if(ButtonLightPin[i]>=0) {
            pinMode(ButtonLightPin[i], OUTPUT);
            digitalWrite(ButtonLightPin[i],0);
        }
        ButtonStatus[i] = 0;
        ButtonLastStatusChangeTime[i] = 0;

        RelayPinStatus[i] = 0;
        ButtonLightMoveTo[i]=0;
        ButtonLightPinStatus[i] =0;
        ButtonLightCurrentPosition[i]=0;
        ButtonLightTimeInCurrentPosition[i] = 0;
        RelayOnTime[i] = 0;
        Stat.relay_switch[i] = 0;
        Stat.relay_time[i] = 0;
    }

    assert(storage);
    _Storage = storage;
    storage->load(storage->object, "relays", &(this->Stat), sizeof(this->Stat));

    pthread_create(&WorkingThread, NULL, DiaGpio_WorkingThread, this);
    pthread_create(&LedSwitchingThread, NULL, DiaGpio_LedSwitcher, this);
    pthread_setschedprio(WorkingThread, SCHED_FIFO);
    InitializedOk = 1;
}

void DiaGpio_StartAdditionalHandler(DiaGpio *gpio) {
  int foundPin = -1;
  for (int i = PIN_COUNT - 1;i>=0;i--) {
    if (gpio->ButtonPin[i]>=0) {
      foundPin = gpio->ButtonPin[i];
      break;
    }    
  }
  if(foundPin >=0 ) {
    gpio->AdditionalHandler = new PulseHandler(foundPin);
  }
}

void DiaGpio_StopRelays(DiaGpio * gpio) {
    printf("stop relays\n");
    for(int i=0;i<=gpio->MaxRelays;i++) {
        DiaGpio_WriteRelay(gpio, i, 0);
    }
}

void DiaGpio_SetProgram(DiaGpio * gpio, int programNumber, int relayNumber,  int onTime, int offTime) {
    gpio->Programs[programNumber].RelayNum[relayNumber] = relayNumber;
    gpio->Programs[programNumber].OnTime[relayNumber] = onTime;
    gpio->Programs[programNumber].OffTime[relayNumber] = offTime;
}

void DiaGpio_CheckRelays(DiaGpio * gpio, long curTime) {
    assert(gpio);
    if(gpio->CurrentProgram>(int)sizeof(gpio->Programs)) {
        printf("Disabling programs as current program is out of range %d...\n", gpio->CurrentProgram);
        gpio->CurrentProgram = -1;
    }

    if(gpio->CurrentProgram<0) {
        if(!gpio->AllTurnedOff) {
            gpio->AllTurnedOff = 1;
            printf("turning all off\n");
            DiaGpio_StopRelays(gpio);
        }
    } else {
        gpio->AllTurnedOff = 0;
        DiaRelayConfig * config = &gpio->Programs[gpio->CurrentProgram];
        //printf("cur_prog:%d\n", gpio->CurrentProgram);
        for(int i=0;i<PIN_COUNT;i++) {
            if(gpio->RelayPin[i]<0) {
                //printf("skipping %d\n",i);
                continue;
            }

            //printf("cur config: on:%ld, off:%ld\n",config->OnTime[i], config->OffTime[i] );
            if(config->OnTime[i]<=0) {
                if(gpio->RelayPinStatus[i]) {
                    printf("-%d; ontime:%ld status:%d\n", i, config->OnTime[i], gpio->RelayPinStatus[i]);
                    DiaGpio_WriteRelay(gpio, i, 0);
                }
            } else if(config->OffTime[i]<=0) {
                if(!gpio->RelayPinStatus[i]) {
                    DiaGpio_WriteRelay(gpio, i, 1);
                    //printf("+%d\n", i);
                }
            } else {
                //printf("curTime %ld,  nextSwitch %ld \n", curTime, config->NextSwitchTime[i] );
                if(curTime>=config->NextSwitchTime[i]) {
                    if(gpio->RelayPinStatus[i]) {
                        DiaGpio_WriteRelay(gpio, i, 0);
                        config->NextSwitchTime[i] = curTime + config->OffTime[i];
                    } else  {
                        DiaGpio_WriteRelay(gpio, i,1);
                        config->NextSwitchTime[i] = curTime + config->OnTime[i];
                    }
                }
            }
        }
    }
}

int DiaGpio_ReadButton(DiaGpio * gpio, int ButtonNumber) {
  if(ButtonNumber>=0 && ButtonNumber<=gpio->MaxButtons) {
    if(gpio->ButtonPin[ButtonNumber]>=0) {
      return !digitalRead(gpio->ButtonPin[ButtonNumber]);
    }
  }
  return 0;
}

void DiaGpio_ButtonAnimation(DiaGpio * gpio, long curTime) {
	if(gpio->AnimationCode == ONE_BUTTON_ANIMATION) {
		int curStep = curTime/DEFAULT_LIGHTING_TIME_MS;
		int value = 10;
		curStep=curStep%3;
		if(!curStep) {
            value = 0;
		}
		for(int i=0;i<PIN_COUNT;i++) {
			if (i==gpio->AnimationSubCode) {
				DiaGpio_WriteLight(gpio, i, value);
			} else {
				DiaGpio_WriteLight(gpio, i, 0);
			}
		}
	} else if(gpio->AnimationCode == IDLE_ANIMATION) {
		int curStep = curTime / DEFAULT_LIGHTING_TIME_MS;
		curStep=curStep % ((gpio->MaxButtons - 1)*2);
		int iShouldBeOn = curStep + 1;
		if(iShouldBeOn>gpio->MaxButtons) iShouldBeOn = (gpio->MaxButtons * 2) - iShouldBeOn;
		for(int i=0;i<PIN_COUNT;i++) {
            if(i==iShouldBeOn) {
                DiaGpio_WriteLight(gpio, i, 10);
			} else {
                DiaGpio_WriteLight(gpio, i, 0);
			}
		}
	} else if(gpio->AnimationCode == BALANCE_ANIMATION) {
		int curStep = curTime/DEFAULT_LIGHTING_TIME_MS;
		curStep=curStep % (gpio->MaxButtons *2);
		int curHalfStep = curStep - gpio->MaxButtons;
		for(int i=0;i<PIN_COUNT;i++) {
            int iShouldBeOn = 0;
            if(curStep>=i) {
                iShouldBeOn = 10;
            }
            if(curHalfStep>=i) {
                iShouldBeOn = 0;
            }
            DiaGpio_WriteLight(gpio, i, iShouldBeOn);
		}
	} else if (gpio->AnimationCode == FREEZE_ANIMATION) {
    
  }
  else {
        for(int i=0;i<PIN_COUNT;i++) {
            DiaGpio_WriteLight(gpio, i, 0);
		}
  }
}



inline int DiaGpio_Abs(int from, int to) {
    if(to>from) return 1;
    if(to<from) return -1;
    return 0;
}

void * DiaGpio_LedSwitcher(void * gpio)
{
    DiaGpio *Gpio = (DiaGpio *)gpio;
    int step = 0;
    int MagicNumber = DEFAULT_LIGHTING_TIME_MS / 12; //10 brightness levels are existing
    while(Gpio->NeedWorking)
    {
        for(int i=0;i<PIN_COUNT;i++)
        {
            if(Gpio->ButtonPin[i]>=0)
            {
                if(step==MagicNumber)
                {
                    int abs = DiaGpio_Abs(Gpio->ButtonLightCurrentPosition[i], Gpio->ButtonLightMoveTo[i]);
                    Gpio->ButtonLightCurrentPosition[i] = Gpio->ButtonLightCurrentPosition[i] + abs;
                    step = 0;
                }
                int OnTime = Gpio->ButtonLightCurrentPosition[i];
                int OffTime = 10-OnTime;
                if(Gpio->ButtonLightPinStatus[i])
                {
                    if(Gpio->ButtonLightTimeInCurrentPosition[i]>=OnTime && OffTime)
                    {
                        DiaGpio_TurnLED(Gpio, i, 0);
                    }
                } else
                {
                    if(Gpio->ButtonLightTimeInCurrentPosition[i]>=OffTime && OnTime)
                    {
                        DiaGpio_TurnLED(Gpio, i, 1);
                    }
                }
                Gpio->ButtonLightTimeInCurrentPosition[i] = Gpio->ButtonLightTimeInCurrentPosition[i] + 1;
                step++;
            }
        }
        delay(1);
    }
    pthread_exit(NULL);
    return NULL;
}


void * DiaGpio_WorkingThread(void * gpio) {
    DiaGpio *Gpio = (DiaGpio *)gpio;

    long curTime = 0;

    while(Gpio->NeedWorking) {
      delay(1);//This code will run once per ms
      curTime+=1;
      DiaGpio_CheckRelays(Gpio, curTime);

      Gpio->CoinsHandler->Tick();
      Gpio->BanknotesHandler->Tick();
      if(Gpio->AdditionalHandler) {
          Gpio->AdditionalHandler->Tick();
      }

      DiaGpio_ButtonAnimation(Gpio, curTime);

      // noise reduction code ... 
      // not really sure how its working :(
      if(curTime % 25 == 0) {
        for(int i=0;i<=Gpio->MaxButtons;i++) {
          if(DiaGpio_ReadButton(Gpio, i)) {
              if(!Gpio->ButtonStatus[i]) {
                  Gpio->ButtonStatus[i] = 1;
                  Gpio->ButtonLastStatusChangeTime[i] = curTime + 100;
                  Gpio->LastPressedKey = i;
              }
          } else {
            if(Gpio->ButtonStatus[i] && curTime > Gpio->ButtonLastStatusChangeTime[i]) {
              Gpio->ButtonStatus[i] = 0;
            }
          }
        }
      }
    }
    pthread_exit(NULL);
    return NULL;
}
