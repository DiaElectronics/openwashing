#ifndef DIA_DEVICE_MANAGER
#define DIA_DEVICE_MANAGER

#include <list>
#include "dia_device.h"
#include "dia_cardreader.h"
#include <pthread.h>
#define DIAE_DEVICE_MANAGER_NOERROR 0


class DiaDeviceManager {
public:
    int CoinMoney;
    int BanknoteMoney;
    int ElectronMoney;
    int ServiceMoney;
    
    int NeedWorking;
    int isBanknoteReaderFound;
    char * _PortName;
    pthread_mutex_t _DevicesLock = PTHREAD_MUTEX_INITIALIZER;

    DiaCardReader* _CardReader;
    
    std::list<DiaDevice*> _Devices;

    pthread_t WorkingThread;
    DiaDeviceManager();
    ~DiaDeviceManager();
};
void * DiaDeviceManager_WorkingThread(void * manager);
void DiaDeviceManager_ScanDevices(DiaDeviceManager * manager);
void DiaDeviceManager_StartDeviceScan(DiaDeviceManager * manager);
void DiaDeviceManager_FinishDeviceScan(DiaDeviceManager * manager);
void DiaDeviceManager_CheckOrAddDevice(DiaDeviceManager *manager, char * PortName, int isACM);
void DiaDeviceManager_ReportMoney(void *manager, int moneyType, int Money);
void DiaDeviceManager_PerformTransaction(void *manager, int money);
void DiaDeviceManager_AbortTransaction(void *manager);
int DiaDeviceManager_GetTransactionStatus(void *manager);
void DiaDeviceManager_AddCardReader(DiaDeviceManager * manager);

#endif
