#ifndef NV9USBDRIVER_H
#define NV9USBDRIVER_H

#include "dia_device.h"
#define DIA_NV9_NO_ERROR 0
#define DIA_NV9_UNKNOWN_COMMAND 1
#define DIA_NV9_EMPTY_COMMAND 2
#define DIA_NV9_THREAD_ERROR 3
#define DIA_NV9_NULL_PARAMETER 4

#define DIA_NV9_JUSTCREATED 0
#define DIA_NV9_DRIVER_STARTED 1
#define DIA_NV9_DRIVER_OPERATIONAL_STATE 2
#define DIA_NV9_DRIVER__MONEY_ON_DEPOSITE 3
#define DIA_NV9_DRIVER__ERROR_HAPPENED_PLEASE_REINITIALIZE 4

class DiaNv9Usb
{
    public:
    int Money;

    int CurrentMode;
    void (*IncomingMoneyHandler)(void * manager, int moneyType, int newMoney);

    DiaDevice * _Device;

    pthread_t CommandReadingThread;
    pthread_mutex_t MoneyLock = PTHREAD_MUTEX_INITIALIZER;
    int ToBeDeleted;

    DiaNv9Usb(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) );
    ~DiaNv9Usb();
};

int DiaNv9Usb_GetBalance(void * specificDriver);
void DiaNv9Usb_CleanBalance(void * specificDriver, int balanceToClean);
int DiaNv9Usb_GetPressedButton(void * specificDriver);;

void DiaNv9Usb_ProcessIncomingData(void * specificDriver, char * data, int length);//must be empty, not required in current implementation (which is bad and misacrhitecture and needs to be fixed)
void * DiaNv9UsbDriver_CommandReadingThread(void * driverPtr);

void DiaNv9Usb_SendByte(DiaNv9Usb * driver, char byteToSend);
void DiaNv9Usb_TurnOn(DiaNv9Usb * driver);
void DiaNv9Usb_TurnOff(DiaNv9Usb * driver);
int DiaNv9Usb_ProcessCommand(DiaNv9Usb * driver, char currentCommand);

int DiaNv9Usb_StartDriver(void * specficDriver);
int DiaNv9Usb_StopDriver(void * specficDriver);


#endif
