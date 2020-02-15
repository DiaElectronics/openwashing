#include "dia_devicemanager.h"

#include <pthread.h>
#include <string.h>
#include <string>
#include <dirent.h>
#include <stdio.h>
#include "dia_nv9usb.h"
#include "dia_microcoinsp.h"
#include "dia_cardreader.h"
#include <wiringPi.h>
#include "money_types.h"

void DiaDeviceManager_AddCardReader(DiaDeviceManager * manager) {
    printf("Card reader added to Device Manager\n");
    manager->_CardReader = new DiaCardReader(manager, DiaDeviceManager_ReportMoney);
}

void DiaDeviceManager_StartDeviceScan(DiaDeviceManager * manager)
{
    pthread_mutex_lock(&(manager->_DevicesLock));
    for (std::list<DiaDevice*>::iterator it=manager->_Devices.begin(); it != manager->_Devices.end(); ++it)
    {
        (*it)->_CheckStatus = DIAE_DEVICE_STATUS_INITIAL;
    }
    pthread_mutex_unlock(&(manager->_DevicesLock));
}

void DiaDeviceManager_CheckOrAddDevice(DiaDeviceManager *manager, char * PortName, int isACM) {
    pthread_mutex_lock(&(manager->_DevicesLock));

    int devInList = 0;
    for (std::list<DiaDevice*>::iterator it=manager->_Devices.begin(); it != manager->_Devices.end(); ++it)
    {
        if(strcmp(PortName, (*it)->_PortName ) == 0)
        {
            devInList = 1;
            (*it)->_CheckStatus = DIAE_DEVICE_STATUS_IN_LIST;
        }
    }
    if(!devInList)
    {
        DiaDevice * dev = new DiaDevice(PortName);
        printf("Device manager found device: %s\n", PortName);
        dev->Manager = manager;
        dev->_CheckStatus = DIAE_DEVICE_STATUS_JUST_ADDED;
        dev->Open();

        if (isACM)
        {
            printf("\nFOUND nv9\n\n");
            DiaNv9Usb * newNv9 = new DiaNv9Usb(dev, DiaDeviceManager_ReportMoney);
            DiaNv9Usb_StartDriver(newNv9);
            manager->_Devices.push_back(dev);
        } else {
            printf("found coin acceptor\n");
           
            int res = DiaMicroCoinSp_Detect(dev);
            if (res)
            {
                DiaMicroCoinSp * newMicroCoinSp = new DiaMicroCoinSp(dev, DiaDeviceManager_ReportMoney);
                DiaMicroCoinSp_StartDriver(newMicroCoinSp);
                manager->_Devices.push_back(dev);
            } else {
                printf("coin acceptor check failed\n");
            }
        }
    }
    pthread_mutex_unlock(&(manager->_DevicesLock));
}

void DiaDeviceManager_FinishDeviceScan(DiaDeviceManager * manager)
{
	return;
    pthread_mutex_lock(&(manager->_DevicesLock));

    for (std::list<DiaDevice*>::iterator it=manager->_Devices.begin(); it != manager->_Devices.end(); ++it)
    {
        printf("lst dev: %s\n", (*it)->_PortName);
        if((*it)->_CheckStatus==DIAE_DEVICE_STATUS_INITIAL)
        {
			DiaDevice * dev = (*it);
			dev->NeedWorking = 0;
			dev->_PortName[0] = 0;
			DiaDevice_CloseDevice(dev);
            //remove device from the list;
        }
    }

    pthread_mutex_unlock(&(manager->_DevicesLock));
}

void DiaDeviceManager_ScanDevices(DiaDeviceManager * manager)
{
    if(manager == NULL)
    {
        return;
    }
    struct dirent *entry;
	DIR * dir;
    DiaDeviceManager_StartDeviceScan(manager);
    if((dir = opendir("/dev"))!=NULL)
	{
        while((entry = readdir(dir))!=NULL)
		{
		    //if(strstr(entry->d_name,"ttyUSB")||strstr(entry->d_name,"ttyACM"))
            if(strstr(entry->d_name,"ttyACM"))
            {
                char buf[1024];
                snprintf(buf, 1023, "/dev/%s", entry->d_name);
                DiaDeviceManager_CheckOrAddDevice(manager, buf, 1);
            } else if (strstr(entry->d_name,"ttyUSB"))
            {
                char buf[1024];
                snprintf(buf, 1023, "/dev/%s", entry->d_name);
                DiaDeviceManager_CheckOrAddDevice(manager, buf, 0);
            }
        }
        closedir(dir);
    }
    DiaDeviceManager_FinishDeviceScan(manager);
}

DiaDeviceManager::DiaDeviceManager()
{
    NeedWorking = 1;
    CoinMoney = 0;
    BanknoteMoney = 0;
    ElectronMoney = 0;
    ServiceMoney = 0;
    pthread_create(&WorkingThread, NULL, DiaDeviceManager_WorkingThread, this);
}

DiaDeviceManager::~DiaDeviceManager() {
}

void DiaDeviceManager_ReportMoney(void *manager, int moneyType, int money) {
    printf("Entered report money\n");
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;
    if (moneyType == DIA_BANKNOTES) {
        Manager->BanknoteMoney += money;
    } else if (moneyType == DIA_COINS) {
        Manager->CoinMoney += money;
    } else if (moneyType == DIA_ELECTRON) {
        Manager->ElectronMoney += money;
    } else if (moneyType == DIA_SERVICE) {
        Manager->ServiceMoney += money;
    } else {
        printf("ERROR: Unknown money type %d\n", money);
    }
    printf("Money: %d\n", money);
}

void * DiaDeviceManager_WorkingThread(void * manager)
{
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;

    while(Manager->NeedWorking)
    {
        DiaDeviceManager_ScanDevices(Manager);
        delay(100);
    }
    return 0;
}

void DiaDeviceManager_PerformTransaction(void *manager, int money) {
    if (manager == NULL)
    {
        printf("DiaDeviceManager Perform Transaction got NULL driver\n");
	    return;
    }
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;
    printf("DiaDeviceManager got Perform Transaction, money = %d\n", money);
    DiaCardReader_PerformTransaction(Manager->_CardReader, money);
}

void DiaDeviceManager_AbortTransaction(void *manager) {
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;
    if(manager == NULL)
    {
        printf("DiaDeviceManager Abort Transaction got NULL driver\n");
        return;
    }
    DiaCardReader_AbortTransaction(&Manager->_CardReader);
}

int DiaDeviceManager_GetTransactionStatus(void *manager) {
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;
    if(manager == NULL)
    {
        printf("DiaDeviceManager Get Transaction Status got NULL driver\n");
        return -1;
    }
    return DiaCardReader_GetTransactionStatus(&Manager->_CardReader);
}
