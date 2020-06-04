#ifndef DIA_CCNET_H
#define DIA_CCNET_H

#include "dia_device.h"

class DiaCcnet
{
    public:
    int Money;

    int CurrentMode;
    void (*IncomingMoneyHandler)(void * manager, int moneyType, int newMoney);

    DiaDevice * _Device;

    pthread_t CommandReadingThread;
    pthread_t MainThread;
    pthread_mutex_t MoneyLock = PTHREAD_MUTEX_INITIALIZER;
    int ToBeDeleted;

    DiaCcnet(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) );
    ~DiaCcnet();
    
    int StartDevice();
    int GetBanknoteCode();
};

int DiaCcnet_StartDriver(DiaCcnet * banknoteAcceptor);
int DiaCcent_GetCode(uint8_t * buffer, int bytes_read);
int DiaCcnet_SendCcnetAndReadAnswerCode(DiaDevice * device, uint8_t* buffer);
int DiaCcnet_SendCommand(DiaDevice * device, uint8_t *command);
int DiaCcnet_Detect(DiaDevice * device);

#endif
