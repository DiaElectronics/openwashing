#ifndef CARDREADERDRIVER_H
#define CARDREADERDRIVER_H

#include "dia_device.h"
#include <stdio.h>

#define DIA_CARDREADER_NO_ERROR 0
#define DIA_CARDREADER_NULL_PARAMETER 4

class DiaCardReader 
{
public: 
    void (*IncomingMoneyHandler)(void * manager, int moneyType, int newMoney);

    void * _Manager;

    pthread_t ExecuteDriverProgramThread;
    pthread_mutex_t MoneyLock = PTHREAD_MUTEX_INITIALIZER;
    int ToBeDeleted;
    int RequestedMoney;

    DiaCardReader(void * manager, void (*incomingMoneyHandler)(void * cardreader, int moneyType, int newMoney) ) {
        _Manager = manager;
        IncomingMoneyHandler = incomingMoneyHandler;
        RequestedMoney = 0;
        printf("Card Reader created\n");
    }
    
    ~DiaCardReader() {
        ToBeDeleted = 1;
    }
};

void DiaCardReader_AbortTransaction(void * specificDriver);

int DiaCardReader_GetTransactionStatus(void * specificDriver);

void* DiaCardReader_ExecuteDriverProgramThread(void * devicePtr);

int DiaCardReader_PerformTransaction(void * specficDriver, int money);

int DiaCardReader_StopDriver(void * specficDriver);

#endif
