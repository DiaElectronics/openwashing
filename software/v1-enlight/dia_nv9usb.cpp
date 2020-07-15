#include "dia_nv9usb.h"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "dia_device.h"
#include "money_types.h"

int DiaNv9Usb_GetBalance(void * specificDriver)
{
    DiaNv9Usb * driver = (DiaNv9Usb *)specificDriver;
    return driver->Money;
}

void DiaNv9Usb_CleanBalance(void * specificDriver, int balanceToClean) {
    DiaNv9Usb * driver = (DiaNv9Usb *) specificDriver;
    pthread_mutex_lock(&driver->MoneyLock);
    driver->Money = driver->Money - balanceToClean;
    pthread_mutex_unlock(&driver->MoneyLock);
}

void * DiaNv9Usb_CommandReadingThread(void * driverPtr) {
    printf("nv9 thread...\n");
    DiaNv9Usb * driver = (DiaNv9Usb *)driverPtr;
    while(!driver->ToBeDeleted) {
        driver->_Device->ReadPortBytes();
        int size  = driver->_Device->_Bytes_Read;

        for(int i=0;i<size;i++) {
            DiaNv9Usb_ProcessCommand(driver, driver->_Device->_Buf[i]);
        }

        delay(100);
    }
    printf("NV9: Exit from thread\n");
    pthread_exit(NULL);
    return NULL;
}

int DiaNv9Usb_StartDriver(void * specficDriver) {
    DiaNv9Usb * driver = (DiaNv9Usb *) specficDriver;
    DiaNv9Usb_TurnOn(driver);
    int err = pthread_create(&driver->CommandReadingThread, NULL, &DiaNv9Usb_CommandReadingThread, driver);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return 1;
    }
    return DIA_NV9_NO_ERROR;
}

void DiaNv9Usb_SendByte(DiaNv9Usb * driver, char byteToSend)
{
    char buf[2];
    buf[0] = byteToSend;
    buf[1] = 0;
    DiaDevice_WritePort(driver->_Device, buf, 1);
}

void DiaNv9Usb_TurnOn(DiaNv9Usb * driver)
{
	//delay(50);
    //DiaNv9Usb_SendByte(driver, (char)170);
    delay(50);
    DiaNv9Usb_SendByte(driver, (char)184);
	//sleep(50);
    //DiaNv9Usb_SendByte(driver, (char)170);

    driver->CurrentMode = DIA_NV9_DRIVER_OPERATIONAL_STATE;
}

void DiaNv9Usb_TurnOff(DiaNv9Usb * driver)
{
    DiaNv9Usb_SendByte(driver, (char)185);
}

int DiaNv9Usb_StopDriver(void * specficDriver)
{
    if(specficDriver == NULL)
    {
        return DIA_NV9_NULL_PARAMETER;
    }
    DiaNv9Usb * driver = (DiaNv9Usb *) specficDriver;
    driver->ToBeDeleted = 1;
    pthread_join(driver->CommandReadingThread, NULL);
    return DIA_NV9_NO_ERROR;
}

int sumByCodeKZ(int currentCommand) {
    int sum = 0;
    if(currentCommand == 1) {
        sum=200;
    } else if(currentCommand == 2) {
        sum=500;// 50 RUR or 500 KZ
    } else if(currentCommand == 3) {
        sum=1000;
    } else if(currentCommand == 4) {
        sum=2000;
    } else if(currentCommand == 5) {
        sum=5000;
    } else if(currentCommand == 6) {
        sum=10000;
    } else if(currentCommand == 7) {
        sum=20000;
    } else {
        sum = currentCommand;
    }
    return sum;
}


int sumByCodeRU(int currentCommand) {
    int sum = 0;
    if(currentCommand == 1) {
        sum=10;
    } else if(currentCommand == 2) {
        sum=50;
    } else if(currentCommand == 3) {
        sum=100;
    } else if(currentCommand == 4) {
        sum=200;
    } else if(currentCommand == 5) {
        sum=500;
    } else if(currentCommand == 6) {
        sum=1000;
    } else if(currentCommand == 7) {
        sum=2000;
    } else if(currentCommand == 8) {
        sum=5000;
    } else {
        sum = currentCommand;
    }
    return sum;
}

int DiaNv9Usb_ProcessCommand(DiaNv9Usb * driver, char currentCommand) {
    printf("command: %d \n", (int)currentCommand);
    if(currentCommand>=1&&currentCommand<=15)
    {
        driver->CurrentMode = DIA_NV9_DRIVER__MONEY_ON_DEPOSITE;
        int sum=sumByCodeKZ(currentCommand);
        //DiaNv9Usb_SendByte(driver, (char)172);

        if(sum>0) {
            if(driver->IncomingMoneyHandler!=NULL) {
                driver->IncomingMoneyHandler(driver->_Device->Manager,DIA_BANKNOTES, sum);
                printf("reported money: %d\n", sum);
            } else {
                printf("no handler to report: %d\n", sum);
            }
        }
    }
   
    return DIA_NV9_NO_ERROR;
}
