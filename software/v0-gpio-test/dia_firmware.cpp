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
#include "dia_security.h"
#include "dia_storage.h"

#define IDLE 1
#define WORK 2
#define PAUSE 3

#define DIA_VERSION "v0.11"

#define USE_GPIO
#define USE_KEYBOARD

#define UNLOCK_KEY "/home/pi/unlock.key"
#define ID_KEY "/home/pi/id.txt"

double _Balance;
double _PauseSeconds;
int _Mode;
int _SubMode;
DiaGpio *gpio;
int _DisplayNumber;
int _DebugKey;
DiaStore storage;

struct income {
    double totalIncomeCoins;
    double totalIncomeBanknotes;
    double totalIncomeElectron;
    double totalIncomeService;
};

struct income cur_income;

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

        DiaStatusRequest_Send(&_Network, DIA_VERSION,  devName,
        cur_income.totalIncomeCoins, cur_income.totalIncomeBanknotes, cur_income.totalIncomeElectron, cur_income.totalIncomeService);

        storage.Save("relays", &(gpio->Stat), sizeof(gpio->Stat));

        char buf[512];
        int cursor=0;
        for(int i=1;i<9;i++) {
            cursor+=sprintf(&buf[cursor], "%d:%d:%lu;", i, gpio->Stat.relay_switch[i], gpio->Stat.relay_time[i]/1000);
        }
        printf("%s\n",buf);


        DiaTextRequest_Send(&_Network, devName, "RELAYS", buf);

    }
    return 0;
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
    int isOk=storage.IsOk();
    const char * status="OK_REDIS";
    if (!isOk) {
        status = "OK_NOREDIS";
    }

    cur_income.totalIncomeCoins = 0;
    cur_income.totalIncomeBanknotes = 0;
    cur_income.totalIncomeElectron = 0;
    cur_income.totalIncomeService = 0;
    if (isOk) {
        int err = storage.Load("income", &cur_income, sizeof(cur_income));
        if (err) {
            status = "OK_NO_INCOME";
        } else {
            status = "OK_INCOME";
        }
    }
    int f = 1;

    printf("version: %s\n", DIA_VERSION);

    char buf_keys[1024];
    if (!file_exists(UNLOCK_KEY)) {
        printf("your firmware is not activated, please type in your password:");
        scanf("%s", buf_keys);
        const char * res_key = DiaCodeRequest_Send(&_Network, dia_security_get_key(), buf_keys);
        const char * resPtr = strstr(res_key, "\r\n\r\n");

        if(resPtr != 0) {
            printf("%s\n",resPtr+4);
            dia_security_write_file(UNLOCK_KEY, resPtr+4);
        }
    }
    if (file_exists(UNLOCK_KEY)) {
        char unlock_key[1024];
        dia_security_read_file(UNLOCK_KEY, unlock_key, sizeof(unlock_key));
        f = !dia_security_check_key(unlock_key);
    }

    gpio = new DiaGpio();

    storage.Load("relays", &(gpio->Stat), sizeof(gpio->Stat) );

    DiaScreen screen;
    DiaDeviceManager manager;
    SDL_Event event;

    int keypress = 0;

    int lastNumber = 3;
    manager.Money = 0;
    gpio->CoinMoney = 0;

    char buffer[8192];

    strcpy(devName,"NO_ID");
    readFile(ID_KEY, devName);
    printf("device name: %s \n", devName);

    DiaPowerOnRequest_Send(&_Network, DIA_VERSION, status, devName);

    while(!keypress)
    {
        if(f) {
            _Balance = 100;
        }
		//GENERAL STEP FOR ALL MODES
        int curMoney = gpio->CoinMoney;
        if(curMoney>0)
        {
			_Balance+=curMoney;
			cur_income.totalIncomeCoins += curMoney;
			storage.Save("income", &cur_income, sizeof(income));
			//printf("coin %d\n", curMoney);
			 gpio->CoinMoney = 0;
		}
		double bb = f;
		_Balance+=bb;
		curMoney = manager.Money;
		if(curMoney>0)
        {
			_Balance+=curMoney;
			cur_income.totalIncomeBanknotes += curMoney;
			storage.Save("income", &cur_income, sizeof(income));
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
        if(f) {
            _Balance = 100;
        }
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
                            cur_income.totalIncomeService+=10;
                            storage.Save("income", &cur_income, sizeof(income));
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
