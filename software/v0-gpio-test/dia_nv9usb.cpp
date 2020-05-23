#include "dia_nv9usb.h"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "dia_device.h"

DiaNv9Usb::DiaNv9Usb(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) )
{
    _Device = device;
    IncomingMoneyHandler = NULL;
    DiaDevice_SetDriver(device, DiaNv9Usb_CommandReadingThread, this);
    IncomingMoneyHandler = incomingMoneyHandler;
}

int DiaNv9Usb_GetBalance(void * specificDriver)
{
    DiaNv9Usb * driver = (DiaNv9Usb *)specificDriver;
    return driver->Money;
}

void DiaNv9Usb_CleanBalance(void * specificDriver, int balanceToClean)
{
    DiaNv9Usb * driver = (DiaNv9Usb *) specificDriver;
    pthread_mutex_lock(&driver->MoneyLock);
    driver->Money = driver->Money - balanceToClean;
    pthread_mutex_unlock(&driver->MoneyLock);
}

void DiaNv9Usb_CommandReadingThread(void * driverPtr, int size, char * buf)
{
    DiaNv9Usb * driver = (DiaNv9Usb *)driverPtr;
    char buf_cur[1024];
    if(size>=1024)
    {
	size = 1023;
    }
    memcpy(buf_cur, buf, size);
    buf_cur[size] = 0;
    if(driver == NULL)
    {
        printf("empty keyboard driver passed to the thread procedure");
    }
    for(int i=0;i<size;i++)
    {
        DiaNv9Usb_ProcessCommand(driver, buf_cur[i]);
    }
}

int DiaNv9Usb_StartDriver(void * specficDriver)
{
    DiaNv9Usb * driver = (DiaNv9Usb *) specficDriver;
    DiaNv9Usb_TurnOn(driver);

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
//    DiaNv9Usb_SendByte(driver, (char)184);
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

int DiaNv9Usb_ProcessCommand(DiaNv9Usb * driver, char currentCommand)
{
    printf("command: %d \n", (int)currentCommand);
        if(currentCommand>=1&&currentCommand<=6)
        {
            driver->CurrentMode = DIA_NV9_DRIVER__MONEY_ON_DEPOSITE;
            int sum=0;
            if(currentCommand == 1) {
                sum=10;
            } else if(currentCommand == 2) {
                sum=50;
            } else if(currentCommand == 3) {
                sum=100;
            } else if(currentCommand == 4) {
                sum=500;
            } else if(currentCommand == 5) {
                sum=1000;
            } else if(currentCommand == 6) {
                sum=5000;
            } else if(currentCommand == 7) {
                sum=5000;
            }
            //DiaNv9Usb_SendByte(driver, (char)172);

            if(sum>0)
            {
                if(driver->IncomingMoneyHandler!=NULL)
                {
                    driver->IncomingMoneyHandler(driver->_Device->Manager, 0, sum);
                    printf("reported money: %d\n", sum);
                }
                else
                {
                    printf("no handler to report: %d\n", sum);
                }
            }
        }
   
    return DIA_NV9_NO_ERROR;
}
