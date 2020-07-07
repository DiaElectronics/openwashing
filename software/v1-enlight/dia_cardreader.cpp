// Copyright 2020, Roman Fedyashov

#include "./dia_cardreader.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "./dia_device.h"
#include "./money_types.h"
#include <assert.h>
// Task thread
// Reads requested money amount and tries to call an executable,
// which works with card reader hardware
// If success - uses callback to report money
// If fails then sets requested money to 0 and stoppes
void * DiaCardReader_ExecuteDriverProgramThread(void * driverPtr) {
    fprintf(stderr, "Card reader executes program thread...\n");
    if (!driverPtr) {
         fprintf(stderr, "%s", "Card reader driver is empty. Panic!\n");
    }

    DiaCardReader * driver = reinterpret_cast<DiaCardReader *>(driverPtr);

    fprintf(stderr, "reader request %d RUB...\n", driver->RequestedMoney);
    int money_command = driver->RequestedMoney * 100;

    std::string money = std::to_string(money_command);

    std::string commandLine = "./uic_payment_app o1 a" + money + " c643";
    int statusCode = system(commandLine.c_str());

    fprintf(stderr, "Card reader returned status code: %d\n", statusCode);
    if (statusCode == 0 || statusCode == 23040) {
        if (driver->IncomingMoneyHandler != NULL) {
            int sum = driver->RequestedMoney;
            driver->IncomingMoneyHandler(driver->_Manager, DIA_ELECTRON, sum);
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
    DiaCardReader * driver = reinterpret_cast<DiaCardReader *>(specificDriver);

    if (specificDriver == NULL || money == 0) {
        printf("DiaCardReader Perform Transaction got NULL driver\n");
        return DIA_CARDREADER_NULL_PARAMETER;
    }

    printf("DiaCardReader started Perform Transaction, money = %d\n", money);

    driver->RequestedMoney = money;
    printf("Money inside driver: %d\n", driver->RequestedMoney);

    int err = pthread_create(&driver->ExecuteDriverProgramThread,
        NULL,
        DiaCardReader_ExecuteDriverProgramThread,
        driver);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return 1;
    }

    return DIA_CARDREADER_NO_ERROR;
}

// Inner function for task thread destroy
int DiaCardReader_StopDriver(void * specificDriver) {
    if (specificDriver == NULL) {
        printf("DiaCardReader Stop Driver got NULL driver\n");
        return DIA_CARDREADER_NULL_PARAMETER;
    }

    DiaCardReader * driver = reinterpret_cast<DiaCardReader *>(specificDriver);
    driver->RequestedMoney = 0;
    printf("Trying to kill cardreader thread...\n");

    char line[10];
    FILE* cmd = popen("pidof -s uic_payment_app", "r");
    int64_t pid = 0;

    char * res = fgets(line, 10, cmd);
    assert(res);
    pid = strtoul(line, NULL, 10);

    if (pid != 0) {
        std::string kill_line = std::string("kill -INT ") + std::to_string(pid);
        int err = system(kill_line.c_str());
        if (err) {
            printf("can't kill cardreader thread %d\n", err);
        }
    }

    pthread_join(driver->ExecuteDriverProgramThread, NULL);

    printf("Cardreader thread killed\n");
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
// If returned number > 0, thread expects money amount == result number
// If returned 0, then thread is not working (destroyed or not created)
// If returned -1, then error occurred during call
int DiaCardReader_GetTransactionStatus(void * specificDriver) {
    if (specificDriver == NULL) {
        printf("DiaCardReader Get Transaction Status got NULL driver\n");
        return -1;
    }

    DiaCardReader * driver = reinterpret_cast<DiaCardReader *>(specificDriver);
    return driver->RequestedMoney;
}
