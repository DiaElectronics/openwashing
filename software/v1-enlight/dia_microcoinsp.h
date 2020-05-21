#ifndef MICROCOINSPDRIVER_H
#define MICROCOINSPDRIVER_H

#include "dia_device.h"
#define DIA_MCSP_NO_ERROR 0
#define DIA_MCSP_UNKNOWN_COMMAND 1
#define DIA_MCSP_EMPTY_COMMAND 2
#define DIA_MCSP_THREAD_ERROR 3
#define DIA_MCSP_NULL_PARAMETER 4

#define DIA_MCSP_JUSTCREATED 0
#define DIA_MCSP_DRIVER_STARTED 1
#define DIA_MCSP_DRIVER_OPERATIONAL_STATE 2
#define DIA_MCSP_DRIVER__MONEY_ON_DEPOSITE 3
#define DIA_MCSP_DRIVER__ERROR_HAPPENED_PLEASE_REINITIALIZE 4

#define MICROCOINSP_DEFAULT_ADDRESS 2
class DiaMicroCoinSp
{
    public:
    int Money;

    int CurrentMode;
    void (*IncomingMoneyHandler)(void * manager, int moneyType, int newMoney);

    DiaDevice * _Device;

    pthread_t CommandReadingThread;
    pthread_t WorkingThread;
    pthread_mutex_t MoneyLock = PTHREAD_MUTEX_INITIALIZER;
    int ToBeDeleted;

	char _Status;
	char _MonetCount;
	char _DeviceAddress;
    char _BufToSend[32];

    DiaMicroCoinSp(DiaDevice * device, void (*incomingMoneyHandler)(void * coinsp, int moneyType, int newMoney) );
    ~DiaMicroCoinSp() {
        
    }
};

int DiaMicroCoinSp_Detect(DiaDevice * device);
int DiaMicroCoinSp_GetPressedButton(void * specificDriver);
void DiaMicroCoinSp_ProcessIncomingData(void * specificDriver, char * data, int length);
void DiaMicroCoinSp_CommandReadingThread(void * driverPtr, int size, char * buf);
long DiaMicroCoinSp_CheckMonet(DiaMicroCoinSp * coinAcceptor);
void DiaMicroCoinSp_PrintBuffer(char *Buf, int bufLength);
void DiaMicroCoinSp_StartDriver(DiaMicroCoinSp * coinAcceptor);
int DiaMicroCoinSp_SendRequest(DiaDevice * device, char * buf , char additionalBytesCount, char cmd);
void DiaMicroCoinSp_SendRequestRaw(DiaDevice * device, char * buf , char additionalBytesCount, char cmd);
int DiaMicroCoinSp_GetAnswerRaw(DiaDevice * device);
void *DiaMicroCoinSp_WorkingThread(void *link);
#endif
