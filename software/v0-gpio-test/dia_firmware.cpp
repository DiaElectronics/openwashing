#include <stdio.h>
#include <wiringPi.h>

#include "dia_gpio.h"
#include "dia_screen.h"
#include "dia_devicemanager.h"
#include "request_collection.h"
#include "dia_network.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define IDLE 1
#define WORK 2
#define PAUSE 3

#define DIA_VERSION "v0.03"

//#define USE_GPIO
#define USE_KEYBOARD

double _Balance;
double _PauseSeconds;
int _Mode;
int _SubMode;
DiaGpio *gpio;
int _DisplayNumber;
int _DebugKey;

double totalIncomeCoins;
double totalIncomeBanknotes;
double totalIncomeElectron;
double totalIncomeService;

DiaNetwork _Network;

int minuteCount;
char devName[128];

inline int SendStatusRequest() {
    minuteCount++;
    if(minuteCount<0) {
        printf("Memory corruption on minutecount\n");
        minuteCount=0;
    }
    if(minuteCount>600) {
        minuteCount= 0;
        printf("sending status\n");

        DiaStatusRequest_Send(&_Network, DIA_VERSION,  devName, totalIncomeCoins, totalIncomeBanknotes, totalIncomeElectron, totalIncomeService);
    }
}

int GetKey()
{
    int key =0;
#ifdef USE_GPIO
    key = DiaGpio_GetLastKey(gpio);
#endif

#ifdef USE_KEYBOARD
    if(_DebugKey!=0) key = _DebugKey;
    _DebugKey = 0;
#endif
    if(key == 7) key = 6;
    return key;
}

void LoopIdle()
{
	if(_Balance<=0.01)
	{
		gpio->CurrentProgram = -1;
	} else
	{
		gpio->CurrentProgram = 6;
	}
	int key = GetKey();
	int curMoney = ceil(_Balance);
	_DisplayNumber = curMoney;
	gpio->AnimationCode = IDLE_ANIMATION;
	if(curMoney>0)
	{
		if(key == 7 || (key>0 && key<=5))
		{
			_PauseSeconds = 120;
			_Mode = 2;
			_SubMode = key;
		}
	}
}

void LoopWorking()
{
	gpio->AnimationCode = ONE_BUTTON_ANIMATION;
	gpio->AnimationSubCode = _SubMode;
	if(_SubMode < 0)
	{
		_SubMode = 0;
	}
	if(_SubMode>=PIN_COUNT)
	{
		_SubMode = PIN_COUNT - 1;
	}
	double Price = 18;
	Price = gpio->Programs[_SubMode].Price;

	int curMoney = ceil(_Balance);
	if(_Balance>0)
	{
		gpio->CurrentProgram = _SubMode;
		_Balance -=Price * 0.001666666666;
	}
	else
	{
		gpio->CurrentProgram = -1;
		_Balance = 0;
		_Mode = 1;
	}

	int key = GetKey();
	if(key == 7 || (key>0 && key<=5))
	{
		_Mode = 2;
		_SubMode = key;
	} else if (key == 6)
	{
		_Mode = 3;
	}

	_DisplayNumber = curMoney;
}

void LoopPause()
{
	gpio->CurrentProgram = 6;
	gpio->AnimationCode = IDLE_ANIMATION;
	int key = GetKey();
	int curSecs = ceil(_PauseSeconds);
	int curMoney = ceil(_Balance);
	if(curSecs>0)
	{
		_PauseSeconds-=0.1;
		_DisplayNumber = curSecs;
	}
	else
	{
		_Balance -=18 * 0.001666666666;
		_DisplayNumber=_Balance;
	}

	if(_Balance<0)
	{
		_Balance=0;
		_Mode = 1;
	}
	else
	{
		if(key == 7 || (key>0 && key<=5))
		{
			_Mode = 2;
			_SubMode = key;
		}
	}
}

int readFile(const char * fileName, char * buf) {
    FILE *fp;
    fp = fopen(fileName, "r"); // read mode

   if (fp == NULL)
   {
      return 1;
   }


char key;
    int cursor = 0;
    while((key = fgetc(fp)) != EOF) {
        if (key!='\n' && key!='\r') {
            buf[cursor] = key;
            cursor++;
            if (cursor>64) {
                break;
            }
        } else {

            break;
        }
    }
    buf[cursor] = 0;

   fclose(fp);
   return 0;
}


int main (int argc, char ** argv) {
    _Mode = 1;
    _SubMode = 0;
    _Balance = 0;
    _DebugKey = 0;
    _PauseSeconds = 0;
    minuteCount = 0;

    totalIncomeCoins = 0;
    totalIncomeBanknotes = 0;
    totalIncomeElectron = 0;
    totalIncomeService = 0;

    gpio = new DiaGpio();


    DiaScreen screen;
    DiaDeviceManager manager;
    SDL_Event event;

    int keypress = 0;

    int lastNumber = 3;
    manager.Money = 0;
    gpio->CoinMoney = 0;

    char buffer[8192];

    strcpy(devName,"NO_ID");
    readFile("./id.txt", devName);
    printf("device name: %s \n", devName);

    DiaPowerOnRequest_Send(&_Network, DIA_VERSION, "OK", devName);

    while(!keypress)
    {
		//GENERAL STEP FOR ALL MODES
        int curMoney = gpio->CoinMoney;
        if(curMoney>0)
        {
			_Balance+=curMoney;
			totalIncomeCoins += curMoney;
			printf("coin %d\n", curMoney);
			gpio->CoinMoney = 0;
		}
		curMoney = manager.Money;
		if(curMoney>0)
        {
			_Balance+=curMoney;
			totalIncomeBanknotes += curMoney;
			printf("bank %d\n", curMoney);
			manager.Money = 0;
		}
		curMoney = ceil(_Balance);
		int curSeconds = ceil(_PauseSeconds);

        if(_DisplayNumber != lastNumber)
        {
            screen.Number = _DisplayNumber;
            DiaScreen_DrawScreen(&screen);
            lastNumber = _DisplayNumber;
        }

        if(_Mode == 1)
        {
			LoopIdle();
		}
		if(_Mode == 2)
		{
			LoopWorking();
		}
		if(_Mode == 3)
		{
			LoopPause();
		}


        delay(100);
        SendStatusRequest();
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    keypress = 1;
                break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_UP:
                            _Balance+=10;
                            totalIncomeService+=10;
                            printf("UP\n"); fflush(stdout);
                            break;
                        case SDLK_1:
                            _DebugKey = 1;
                            printf("1\n"); fflush(stdout);
                            break;
                        case SDLK_2:
                            _DebugKey = 2;
                            printf("2\n"); fflush(stdout);
                            break;
                        case SDLK_3:
                            _DebugKey = 3;
                            printf("3\n"); fflush(stdout);
                            break;
                        case SDLK_4:
                            _DebugKey = 4;
                            printf("4\n"); fflush(stdout);
                            break;
                        case SDLK_5:
                            _DebugKey = 5;
                            printf("5\n"); fflush(stdout);
                            break;
                        case SDLK_6:
                            _DebugKey = 6;
                            printf("6\n"); fflush(stdout);
                            break;
                        case SDLK_7:
                            _DebugKey = 7;
                            printf("7\n"); fflush(stdout);
                            break;
                        default:
                            keypress = 1;
                            printf("quitting...");
                            break;
                    }
                break;
            }
        }
    }
    return 0;
}
