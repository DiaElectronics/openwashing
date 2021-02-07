#ifndef VENDOTEKDRIVER_H
#define VENDOTEKDRIVER_H

#include "dia_device.h"
#include "./vendotek/vendotek.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>


#define DIA_VENDOTEK_NO_ERROR 0
#define DIA_VENDOTEK_NULL_PARAMETER 4

typedef struct payment_opts_s {
    vtk_t     *vtk;
    vtk_msg_t *mreq;
    vtk_msg_t *mresp;
    int        ping;
    int        timeout;
    int        verbose;

    ssize_t    evnum;
    char      *evname;
    ssize_t    prodid;
    char      *prodname;
    ssize_t    price;
} payment_opts_t;

class DiaVendotek 
{
public: 
    void (*IncomingMoneyHandler)(void * manager, int moneyType, int newMoney);

    void * _Manager;

    pthread_t ExecuteDriverProgramThread;
    pthread_mutex_t MoneyLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_t ExecutePingThread;
    pthread_mutex_t OperationLock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t StateLock = PTHREAD_MUTEX_INITIALIZER;
    int ToBeDeleted = 0;
    int RequestedMoney;
    int Available = 0;
    int PaymentStage = 0;
    char *Host = new char;
    char *Port = new char;
    vtk_t *_Vtk = NULL;
    payment_opts_t *_PaymentOpts = NULL;
    DiaVendotek(void * manager, void (*incomingMoneyHandler)(void * vendotek, int moneyType, int newMoney), const char *host, const char *port) {
        _Manager = manager;
        IncomingMoneyHandler = incomingMoneyHandler;
        RequestedMoney = 0;
        strcpy(Host,host);
        strcpy(Port,port);
        printf("Card Reader created\n");
    }
    
    ~DiaVendotek() {
        ToBeDeleted = 1;
    }
};

void DiaVendotek_AbortTransaction(void * specificDriver);

int DiaVendotek_GetTransactionStatus(void * specificDriver);

void* DiaVendotek_ExecuteDriverProgramThread(void * devicePtr);

int DiaVendotek_PerformTransaction(void * specficDriver, int money);

int DiaVendotek_StopDriver(void * specficDriver);

int DiaVendotek_StartPing(void * specificDriver);
int DiaVendotek_GetAvailableStatus(void * specificDriver);

#endif
