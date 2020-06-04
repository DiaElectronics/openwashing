#include <stdio.h>
#include <wiringPi.h>

#include "dia_gpio.h"
#include "pthread.h"

DiaGpio::~DiaGpio()
{

}

int DiaGpio_GetLastKey(DiaGpio * gpio)
{
    if(gpio->LastPressedKey>=0) {
        int res = gpio->LastPressedKey;
        gpio->LastPressedKey = 0;
        return res;
    }
    return 0;
}

void DiaGpio_TurnLED(DiaGpio * gpio, int lightNumber, int value)
{
    if(value<=0)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }
    if(gpio->InitializedOk)
    {
        if(lightNumber>=0 && lightNumber<PIN_COUNT && gpio->ButtonLightPin[lightNumber]>=0)
        {
            digitalWrite(gpio->ButtonLightPin[lightNumber], value);
            gpio->ButtonLightPinStatus[lightNumber] = value;
            gpio->ButtonLightTimeInCurrentPosition[lightNumber] = 0;
        }
    }
}

void DiaGpio_WriteLight(DiaGpio * gpio, int lightNumber, int value)
{
    if(lightNumber>=0 && lightNumber<PIN_COUNT)
    {
        gpio->ButtonLightMoveTo[lightNumber] = value;
    }
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
                Gpio->ButtonLightTimeInCurrentPosition[i] = Gpio->ButtonLightTimeInCurrentPosition[i]+=1;
                step++;
            }
        }
        delay(1);
    }
    pthread_exit(NULL);
    return NULL;
}


void DiaGpio_WriteRelay(DiaGpio * gpio, int relayNumber, int value)
{
    if(value<=0)
    {
        value = 0;
    }
    else
    {
        value = 1;
    }
    if(gpio->InitializedOk)
    {
        if(relayNumber>=0 && relayNumber<PIN_COUNT && gpio->RelayPin[relayNumber]>=0)
        {
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

            digitalWrite(gpio->RelayPin[relayNumber], value);
            gpio->RelayPinStatus[relayNumber] = value;
        }
    }
}

DiaGpio::DiaGpio()
{
    InitializedOk = 0;
    NeedWorking = 1;
    CoinMoney = 0;
    if (wiringPiSetup () == -1) return ;

    ButtonPin[0] = -1;
    ButtonPin[1] = 13;
    ButtonPin[2] = 14;
    ButtonPin[3] = 21;
    ButtonPin[4] = 22;
    ButtonPin[5] = 23;
    ButtonPin[6] = 24;
    ButtonPin[7] = 25;
    ButtonPin[8] = -1;

    ButtonLightPin[0] = -1;
    ButtonLightPin[1] = 8;
    ButtonLightPin[2] = 9;
    ButtonLightPin[3] = 7;
    ButtonLightPin[4] = 0;
    ButtonLightPin[5] = 2;
    ButtonLightPin[6] = 3;
    ButtonLightPin[7] = 12;
    ButtonLightPin[8] = -1;

    RelayPin[0] = -1;
    RelayPin[1] = 15;
    RelayPin[2] = 16;
    RelayPin[3] = 1;
    RelayPin[4] = -1;
    RelayPin[5] = 4;
    RelayPin[6] = -1;
    RelayPin[7] = 5;
    RelayPin[8] = 6;

    CoinPin = 29;
    DoorPin = 28;

    DiaGpio * gpio = this;

    //preliminary wash
    gpio->Programs[1].Price = 25;
    DiaGpio_SetProgram(gpio, 1, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 1, 3,  20, 450);
    DiaGpio_SetProgram(gpio, 1, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 1, 8,  1000, 0);

    //Soap
    gpio->Programs[2].Price = 25;
    DiaGpio_SetProgram(gpio, 2, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 2, 3,  135, 350);
    DiaGpio_SetProgram(gpio, 2, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 2, 8,  1000, 0);

    //Rinse
    gpio->Programs[3].Price = 20;
    DiaGpio_SetProgram(gpio, 3, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 3, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 3, 8,  1000, 0);

    //Wax
    gpio->Programs[4].Price = 20;
    DiaGpio_SetProgram(gpio, 4, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 4, 5,  135, 350);
    DiaGpio_SetProgram(gpio, 4, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 4, 8,  1000, 0);

    //Osmosian
    gpio->Programs[5].Price = 20;
    DiaGpio_SetProgram(gpio, 5, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 5, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 5, 8,  1000, 0);

    //Pause
    gpio->Programs[6].Price = 20;
    DiaGpio_SetProgram(gpio, 6, 8,  1000, 0);

    //Soap Premium
    gpio->Programs[7].Price = 50;
    DiaGpio_SetProgram(gpio, 7, 1,  1000, 0);
    DiaGpio_SetProgram(gpio, 7, 3,  250, 250);
    DiaGpio_SetProgram(gpio, 7, 7,  1000, 0);
    DiaGpio_SetProgram(gpio, 7, 8,  1000, 0);

    pinMode(CoinPin, INPUT);

    for(int i=0;i<PIN_COUNT;i++)
    {
        if(RelayPin[i] >= 0)
        {
            pinMode(RelayPin[i], OUTPUT);
        }
        if(ButtonPin[i]>=0)
        {
            pinMode(ButtonPin[i], INPUT);
        }
        if(ButtonLightPin[i]>=0)
        {
            pinMode(ButtonLightPin[i], OUTPUT);
        }
        RelayPinStatus[i] = 0;
        ButtonLightMoveTo[i]=0;
        ButtonLightCurrentPosition[i]=0;
        ButtonLightTimeInCurrentPosition[i] = 0;
        RelayOnTime[i] = 0;
        Stat.relay_switch[i] = 0;
        Stat.relay_time[i] = 0;
    }

    pthread_create(&WorkingThread, NULL, DiaGpio_WorkingThread, this);
    pthread_setschedprio(WorkingThread, SCHED_FIFO);
    pthread_create(&LedSwitchingThread, NULL, DiaGpio_LedSwitcher, this);
    InitializedOk = 1;
}



void DiaGpio_StopRelays(DiaGpio * gpio)
{
    for(int i=0;i<9;i++)
    {
        DiaGpio_WriteRelay(gpio, i, 0);
    }
}

void DiaGpio_Test(DiaGpio * gpio)
{
    DiaGpio_WriteRelay(gpio, 1, 1);
    DiaGpio_WriteRelay(gpio, 7, 1);
    DiaGpio_WriteRelay(gpio, 8, 1);
    delay(10000);
    DiaGpio_StopRelays(gpio);
    delay(20000);
}

void DiaGpio_SetProgram(DiaGpio * gpio, int programNumber, int relayNumber,  int onTime, int offTime)
{
    gpio->Programs[programNumber].RelayNum[relayNumber] = relayNumber;
    gpio->Programs[programNumber].OnTime[relayNumber] = onTime;
    gpio->Programs[programNumber].OffTime[relayNumber] = offTime;
}

void DiaGpio_CheckRelays(DiaGpio * gpio, int curTime)
{
    if(gpio->CurrentProgram<0)
    {
        if(!gpio->AllTurnedOff)
        {
            gpio->AllTurnedOff = 1;
            DiaGpio_StopRelays(gpio);
        }
    } else
    {
        DiaRelayConfig * config = &gpio->Programs[gpio->CurrentProgram];
        for(int i=0;i<PIN_COUNT;i++)
        {
            if(config->OnTime[i]<=0)
            {
                if(gpio->RelayPinStatus[i])
                {
                    DiaGpio_WriteRelay(gpio, i, 0);
                }
            } else if(config->OffTime[i]<=0)
            {
                if(!gpio->RelayPinStatus[i])
                {
                    DiaGpio_WriteRelay(gpio, i, 1);
                }
            } else
            {
                if(curTime>=config->NextSwitchTime[i])
                {
                    if(gpio->RelayPinStatus[i])
                    {
                        DiaGpio_WriteRelay(gpio, i, 0);
                        config->NextSwitchTime[i] = curTime + config->OffTime[i];
                    }
                    else
                    {
                        DiaGpio_WriteRelay(gpio, i,1);
                        config->NextSwitchTime[i] = curTime + config->OnTime[i];
                    }
                }
            }
        }
        gpio->AllTurnedOff = 0;
    }
}

int DiaGpio_ReadButton(DiaGpio * gpio, int ButtonNumber)
{
    if(ButtonNumber>=0 && ButtonNumber<PIN_COUNT)
    {
        if(gpio->ButtonPin[ButtonNumber]>=0)
        {
            return digitalRead( gpio->ButtonPin[ButtonNumber] );
        }
    }
    return 0;
}

void DiaGpio_CheckCoin(DiaGpio * gpio)
{
    int curState = digitalRead(gpio->CoinPin);
    gpio->CoinStatus[gpio->CoinLoop] = curState;
    gpio->CoinLoop = gpio->CoinLoop + 1;
    //printf("%d",curState);
    if(gpio->CoinLoop>=COIN_TOTAL)
    {
         gpio->CoinLoop=0;
    }
    int curSwitchedOnPins = 0;
    for(int i=0;i < COIN_TOTAL;i++)
    {
         if( gpio->CoinStatus[i])
         {
              curSwitchedOnPins++;
         }
    }
    if(gpio->CoinStatus_)
    {
        if(curSwitchedOnPins<(COIN_TOTAL-COIN_SWITCH))
        {
            gpio->CoinStatus_ = 0;
            printf("%d\n", gpio->CoinMoney);
        }
    } else
    {
        if(curSwitchedOnPins>COIN_SWITCH)
        {
            gpio->CoinMoney = gpio->CoinMoney + 1;
            printf("%d\n", gpio->CoinMoney);
            //printf("k");
            gpio->CoinStatus_ = 1;
        }
    }
}



void DiaGpio_ButtonAnimation(DiaGpio * gpio, long curTime)
{
	if(gpio->AnimationCode == ONE_BUTTON_ANIMATION)
	{
		int curStep = curTime/DEFAULT_LIGHTING_TIME_MS;
		int value = 10;
		curStep=curStep%3;
		if(!curStep)
		{
            value = 0;
		}
		for(int i=0;i<PIN_COUNT;i++)
		{
			if (i==gpio->AnimationSubCode)
			{
				DiaGpio_WriteLight(gpio, i, value);
			}
			else
			{
				DiaGpio_WriteLight(gpio, i, 0);
			}
		}
	}
	else if(gpio->AnimationCode == IDLE_ANIMATION)
	{
		int curStep = curTime / DEFAULT_LIGHTING_TIME_MS;
		curStep=curStep % 12;
		int iShouldBeOn = curStep + 1;
		if(iShouldBeOn>7) iShouldBeOn = 14 - iShouldBeOn;
		for(int i=0;i<PIN_COUNT;i++)
		{
            if(i==iShouldBeOn)
            {
                DiaGpio_WriteLight(gpio, i, 10);
			} else
			{
                DiaGpio_WriteLight(gpio, i, 0);
			}
		}
	}
	else if(gpio->AnimationCode == BALANCE_ANIMATION)
	{
		int curStep = curTime/DEFAULT_LIGHTING_TIME_MS;
		curStep=curStep%14;
		int curHalfStep = curStep - 7;
		for(int i=0;i<PIN_COUNT;i++)
		{
            int iShouldBeOn = 0;
            if(curStep>=i)
            {
                iShouldBeOn = 10;
            }
            if(curHalfStep>=i)
            {
                iShouldBeOn = 0;
            }
            DiaGpio_WriteLight(gpio, i, iShouldBeOn);
		}
	}
}

inline int DiaGpio_Abs(int from, int to)
{
    if(to>from) return 1;
    if(to<from) return -1;
    return 0;
}

void * DiaGpio_WorkingThread(void * gpio)
{
	int buttonPinLastActivated = -1;
    DiaGpio *Gpio = (DiaGpio *)gpio;
    for(int i=0;i<COIN_TOTAL;i++)
    {
        Gpio->CoinStatus[i] = digitalRead(Gpio->CoinPin);
        Gpio->CoinStatus_ = digitalRead(Gpio->CoinPin);
    }
    Gpio->CoinLoop = 0;

    Gpio->curTime = 0;

    while(Gpio->NeedWorking)
    {
        delay(1);//This code will run once per ms
        Gpio->curTime=Gpio->curTime + 1;
        DiaGpio_CheckRelays(Gpio, Gpio->curTime);
        DiaGpio_CheckCoin(Gpio);
        DiaGpio_ButtonAnimation(Gpio, Gpio->curTime);
        if(Gpio->curTime % 50 == 0)
        {
            for(int i=0;i<PIN_COUNT;i++)
            {
                if(DiaGpio_ReadButton(Gpio, i))
                {
					if(buttonPinLastActivated!=i)
					{
						buttonPinLastActivated = i;
						Gpio->LastPressedKey = i;
						break;
					}
                }
                else
                {
					if(buttonPinLastActivated == i)
					{
						buttonPinLastActivated = -1;
					}
				}
            }
        }
    }
    pthread_exit(0);
    return 0;
}
