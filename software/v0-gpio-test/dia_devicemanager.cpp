#include "dia_devicemanager.h"

#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include "dia_nv9usb.h"
#include <wiringPi.h>


void DiaDeviceManager_StartDeviceScan(DiaDeviceManager * manager)
{
    pthread_mutex_lock(&(manager->_DevicesLock));
    for (std::list<DiaDevice*>::iterator it=manager->_Devices.begin(); it != manager->_Devices.end(); ++it)
    {
        (*it)->_CheckStatus = DIAE_DEVICE_STATUS_INITIAL;
    }
    pthread_mutex_unlock(&(manager->_DevicesLock));
}

void DiaDeviceManager_CheckOrAddDevice(DiaDeviceManager *manager, char * PortName)
{
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
        dev->Manager = manager;
        dev->_CheckStatus = DIAE_DEVICE_STATUS_JUST_ADDED;
        printf("found nv9\n");
        DiaNv9Usb * newNv9 = new DiaNv9Usb(dev, DiaDeviceManager_ReportMoney);
        DiaNv9Usb_StartDriver(newNv9);
        DiaNv9Usb_TurnOn(newNv9);
        manager->_Devices.push_back(dev);
    }
    pthread_mutex_unlock(&(manager->_DevicesLock));
}

void DiaDeviceManager_FinishDeviceScan(DiaDeviceManager * manager)
{
	return;
    pthread_mutex_lock(&(manager->_DevicesLock));

	DiaDevice * dev = NULL;
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
                DiaDeviceManager_CheckOrAddDevice(manager, buf);
            }
        }
        closedir(dir);
    }
    DiaDeviceManager_FinishDeviceScan(manager);
}

DiaDeviceManager::DiaDeviceManager()
{
    NeedWorking = 1;
    pthread_create(&WorkingThread, NULL, DiaDeviceManager_WorkingThread, this);
}

DiaDeviceManager::~DiaDeviceManager()
{
}

void DiaDeviceManager_ReportMoney(void *manager, int moneyType, int money)
{
    DiaDeviceManager * Manager = (DiaDeviceManager *) manager;
    Manager->Money +=money;
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
}
