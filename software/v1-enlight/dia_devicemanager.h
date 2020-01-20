#ifndef DIA_DEVICE_MANAGER
#define DIA_DEVICE_MANAGER

#include <list>
#include "dia_device.h"
#include <pthread.h>
#define DIAE_DEVICE_MANAGER_NOERROR 0


class DiaDeviceManager {
public:
    int CoinMoney;
    int BanknoteMoney;
    int ElectronMoney;
    int ServiceMoney;
    
    int NeedWorking;
    char * _PortName;
    pthread_mutex_t _DevicesLock = PTHREAD_MUTEX_INITIALIZER;
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
#endif
