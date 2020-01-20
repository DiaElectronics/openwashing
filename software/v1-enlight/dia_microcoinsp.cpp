#include "dia_microcoinsp.h"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "money_types.h"

DiaMicroCoinSp::DiaMicroCoinSp(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) )
{
    assert(device);
    _Status = 1;
    _MonetCount = 0;
    _Device = device;
    IncomingMoneyHandler = NULL;
    IncomingMoneyHandler = incomingMoneyHandler;
    _DeviceAddress = MICROCOINSP_DEFAULT_ADDRESS;
}

void DiaMicroCoinSp_CommandReadingThread(void * driverPtr, int bufSize, char * buf)
{
    if(0)
    {
        printf("<<");
        DiaMicroCoinSp_PrintBuffer(buf, bufSize);
    }
}

int DiaMicroCoinSp_Detect(DiaDevice * device)
{
    if (device == NULL) return 0;
    int N = DiaMicroCoinSp_SendRequest(device,0, 0, 245);
    device->_Buf[N] = 0;
    if(N<4)
    {
        printf("answer has %d symbols (%d)\n", N, MICROCOINSP_DEFAULT_ADDRESS);
    } else
    {
        char * str = device->_Buf+4;
        printf("answer has %d symbols (%d)(%s)\n", N, MICROCOINSP_DEFAULT_ADDRESS, str);
        if (strstr(str, "Acceptor"))
        {
            return 1;
        }
    }
    return 0;
}

int DiaMicroCoinSp_SendRequest(DiaDevice * device, char * bufToSend, char additionalBytesCount, char cmd) {
    //printf("send request %d\n", additionalBytesCount); fflush(stdout);
    DiaMicroCoinSp_SendRequestRaw(device, bufToSend, additionalBytesCount, cmd);
    return DiaMicroCoinSp_GetAnswerRaw(device);
}

void DiaMicroCoinSp_SendRequestRaw(DiaDevice *device, char * bufToSend, char additionalBytesCount, char cmd) {
    char sum=0;
    device->_Bytes_Read = 0;
    char buf[64];
    buf[0] = MICROCOINSP_DEFAULT_ADDRESS;
    buf[1] = additionalBytesCount;
    buf[2] = 1;
    buf[3] = cmd;
    for(int i = 0;i < additionalBytesCount; i++)
    {
        buf[4 + i] = bufToSend[i];
    }
    for(int i = 0; i < 4+additionalBytesCount; i++)
    {
        sum-=buf[i];
    }
    buf[4+additionalBytesCount] = sum;
    //printf(">> ");
    //DiaMicroCoinSp_PrintBuffer(buf, 5 + additionalBytesCount);
    DiaDevice_WritePort(device, buf, 5 + additionalBytesCount);
}

int DiaMicroCoinSp_GetAnswerRaw(DiaDevice * device)
{    
    usleep(30000);
    
    int bytes_read = read(device->_handler, device->_Buf, sizeof(device->_Buf)-1);
    return bytes_read;
}

void DiaMicroCoinSp_StartDriver(DiaMicroCoinSp * coinAcceptor)
{
    assert(coinAcceptor->_Device);
    DiaDevice * device = coinAcceptor->_Device;
    DiaMicroCoinSp_SendRequest(device,0, 0,1);

    int res=DiaMicroCoinSp_SendRequest(device,0, 0, 229);
    if (res<5) coinAcceptor->_Status = 0;
    coinAcceptor->_MonetCount = device->_Buf[4];

    int err = pthread_create(&coinAcceptor->WorkingThread, NULL, &DiaMicroCoinSp_WorkingThread, coinAcceptor);
    if (err != 0) {
        printf("\ncan't create thread :[%s]", strerror(err));
        return;
    }

    printf("Microcoin SP initialized properly \n");
}

void DiaMicroCoinSp_PrintBuffer(char *Buf, int bufLength)
{
    for(int i = 0; i < bufLength; i++)
    {
        unsigned char key = Buf[i];
        if(key<100)
        {
            printf("0");
        }
        if(key<10)
        {
            printf("0");
        }
        printf("%d ", (int)key);
    }
    printf("\n");
}

void *DiaMicroCoinSp_WorkingThread(void *link)
{
    assert(link);
    DiaMicroCoinSp * coinAcceptor = (DiaMicroCoinSp*)link;
    DiaDevice * device = coinAcceptor->_Device;
    while(device->NeedWorking)
    {
        long coin = DiaMicroCoinSp_CheckMonet(coinAcceptor);
        if(coin>0)
        {
            printf("coin: %ld", coin);
            if(coinAcceptor->IncomingMoneyHandler!=NULL)
            {
                coinAcceptor->IncomingMoneyHandler(coinAcceptor->_Device->Manager, DIA_COINS, coin);
                printf("reported money: %ld\n", coin);
            }
            else
            {
                printf("no handler to report: %ld\n", coin);
            }
        }
        delay(500);
    }
    pthread_exit(NULL);
}

long DiaMicroCoinSp_CheckMonet(DiaMicroCoinSp * coinAcceptor)
{
    assert(coinAcceptor);
    DiaDevice * device = coinAcceptor->_Device;
    assert(device);
    if (coinAcceptor->_Status != 0)
    {
        int res = DiaMicroCoinSp_SendRequest(device,0 ,0, 229);
        if(res > 5)
        {
            if (device->_Buf[4] != coinAcceptor->_MonetCount)
            {
                printf("kkllll\n");
                coinAcceptor->_MonetCount=device->_Buf[4];
                if (device->_Buf[5]==7) return 10;
                if (device->_Buf[5]==10) return 1;
                if (device->_Buf[5]==11) return 1;
                if (device->_Buf[5]==12) return 2;
                if (device->_Buf[5]==13) return 2;
                if (device->_Buf[5]==14) return 5;
                if (device->_Buf[5]==15) return 5;
                if (device->_Buf[5]==16) return 10;
            }
        }
    }
    return 0;
}
