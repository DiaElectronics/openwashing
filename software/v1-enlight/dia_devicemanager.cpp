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
#include <stdexcept>
#include "money_types.h"

void DiaDeviceManager_AddCardReader(DiaDeviceManager * manager) {
    printf("Abstract card reader added to the Device Manager\n");
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

std::string DiaDeviceManager_ExecBashCommand(const char* cmd, int* error) {
    char buffer[128];
    std::string result = "";
    *error = 0;

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        *error = 1;
        return result;
    } 
        
    while (fgets(buffer, sizeof buffer, pipe) != NULL) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

int DiaDeviceManager_CheckNV9(char* PortName) {
    printf("\nChecking port %s for NV9 device...\n", PortName);

    int error = 0;
    std::string bashOutput = DiaDeviceManager_ExecBashCommand("ls -l /dev/serial/by-id", &error);
    if (error) {
        printf("Error while reading info about serial devices, NV9 check failed\n");
        return 0;
    }

    std::string portName = std::string(PortName);
    size_t maxDiff = 50;

    // Get short name of port, for instance:
    //   /dev/ttyACM0 ==> /ttyACM0
    std::string toCut = portName.substr(0, 4);

    if (toCut != std::string("/dev")) {
        printf("Invlaid port name in NV9 device check: %s\n", PortName);
        return 0;
    }

    std::string shortPortName = portName.substr(4, 8);
    size_t devicePortPosition = bashOutput.find(shortPortName);

    // Check existance of port in list
    if (devicePortPosition != std::string::npos) {
        size_t deviceNamePosition = bashOutput.find("NV9USB");

        // Compare distance between positions with maxDiff const
        if (deviceNamePosition != std::string::npos && 
            devicePortPosition - deviceNamePosition < maxDiff) {
                return 1;
            } 
    }

    return 0;
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
        if (isACM && !manager->isBanknoteReaderFound) {
            if (DiaDeviceManager_CheckNV9(PortName)) {
                printf("\nFound NV9 on port %s\n\n", PortName);
                DiaDevice * dev = new DiaDevice(PortName);

                dev->Manager = manager;
                dev->_CheckStatus = DIAE_DEVICE_STATUS_JUST_ADDED;
                dev->Open();
                DiaNv9Usb * newNv9 = new DiaNv9Usb(dev, DiaDeviceManager_ReportMoney);
                DiaNv9Usb_StartDriver(newNv9);
                manager->_Devices.push_back(dev);
                manager->isBanknoteReaderFound = 1;
            }
        }
        if (!isACM) { 
            printf("\nChecking port %s for MicroCoinSp...\n", PortName);
            DiaDevice * dev = new DiaDevice(PortName);

            dev->Manager = manager;
            dev->_CheckStatus = DIAE_DEVICE_STATUS_JUST_ADDED;
            dev->Open();

            int res = DiaMicroCoinSp_Detect(dev);
            if (res)
            {
                printf("\nFound MicroCoinSp on port %s\n\n", PortName);
                DiaMicroCoinSp * newMicroCoinSp = new DiaMicroCoinSp(dev, DiaDeviceManager_ReportMoney);
                DiaMicroCoinSp_StartDriver(newMicroCoinSp);
                manager->_Devices.push_back(dev);
            } else {
                printf("MicroCoinSp check failed\n");
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
    isBanknoteReaderFound = 0;
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
