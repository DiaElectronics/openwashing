#include "dia_cardreader.h"
#include "stdio.h"
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include "dia_device.h"
#include "money_types.h"

// Task thread
// Reads requested money amount and tries to call an executable, which works with card reader hardware
// If success - uses callback to report money
// If fail - sets requested money to 0 and stoppes
void * DiaCardReader_ExecuteDriverProgramThread(void * driverPtr) {
    printf("Card reader execute program thread...\n");

    DiaCardReader * driver = (DiaCardReader *)driverPtr;

    printf("Transaction requested for %d RUB...\n", driver->RequestedMoney);
    std::string commandLine = "./uic_payment_app o1 a0 c" + std::to_string(driver->RequestedMoney);
    int statusCode = system(commandLine.c_str());

    printf("Card reader returned status code: %d\n", statusCode);
    if (statusCode == 0) {
        if(driver->IncomingMoneyHandler != NULL) {
            driver->IncomingMoneyHandler(driver->_Manager, DIA_ELECTRON, driver->RequestedMoney);
            printf("Reported money: %d\n", driver->RequestedMoney);
        } else {
            printf("No handler to report: %d\n", driver->RequestedMoney);
        }
    }         

    pthread_mutex_lock(&driver->MoneyLock);
    driver->RequestedMoney = 0;
    pthread_mutex_unlock(&driver->MoneyLock);
        
    pthread_exit(NULL);
    return NULL;
}

// Entry point function
// Creates task thread with requested parameter (money amount) and exits
int DiaCardReader_PerformTransaction(void * specificDriver, int money) {
    DiaCardReader * driver = (DiaCardReader *) specificDriver;

    if(specificDriver == NULL || money == 0)
    {
        printf("DiaCardReader Perform Transaction got NULL driver\n");
        return DIA_CARDREADER_NULL_PARAMETER;
    }

    printf("DiaCardReader started Perform Transaction, money = %d\n", money);
    pthread_mutex_lock(&driver->MoneyLock);
    driver->RequestedMoney = money;
    printf("Money inside driver: %d\n", driver->RequestedMoney);

    int err = pthread_create(&driver->ExecuteDriverProgramThread, NULL, DiaCardReader_ExecuteDriverProgramThread, driver);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return 1;
    }
    pthread_mutex_unlock(&driver->MoneyLock);
    
    return DIA_CARDREADER_NO_ERROR;
}

// Inner function for task thread destroy
int DiaCardReader_StopDriver(void * specificDriver) {
    if (specificDriver == NULL) {
        printf("DiaCardReader Stop Driver got NULL driver\n");
        return DIA_CARDREADER_NULL_PARAMETER;
    }

    DiaCardReader * driver = (DiaCardReader *) specificDriver;
    driver->RequestedMoney = 0;
    pthread_join(driver->ExecuteDriverProgramThread, NULL);
    return DIA_CARDREADER_NO_ERROR;
}

// API function for task thread destory
void DiaCardReader_AbortTransaction(void * specificDriver) {
    if (specificDriver == NULL) {
        printf("DiaCardReader Abort Transaction got NULL driver\n");
        return;
    }
    DiaCardReader_StopDriver(specificDriver);
}

// Get task thread status function
// If returns result number > 0, then thread is working on money amount == result number
// If returns 0, then thread is not working (destroyed or not created)
// If returns -1, then error occurred during call
int DiaCardReader_GetTransactionStatus(void * specificDriver) {
    if (specificDriver == NULL) {
        printf("DiaCardReader Get Transaction Status got NULL driver\n");
        return -1;
    }

    DiaCardReader * driver = (DiaCardReader *) specificDriver;
    return driver->RequestedMoney;
}
