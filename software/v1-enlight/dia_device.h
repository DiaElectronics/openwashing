#ifndef DIA_DEVICE_H
#define DIA_DEVICE_H

#include <termios.h>
#include <stdint.h>
#include <pthread.h>
#include <event.h>
#include <wiringPi.h>
#include <unistd.h>


#define DIAE_DEVICE_PASSED_MONEY 1
#define DIAE_DEVICE_PASSED_KEY 2
#define DIAE_DEVICE_PASSED_COMMAND 3

#define DIAE_DEVICE_TYPE_UKNOWN 0
#define DIAE_DEVICE_TYPE_KEYBOARD 1
#define DIAE_DEVICE_TYPE_MONEY 2


#define DIAE_DEVICE_BUFFER_SIZE 16384

#define DIAE_DEVICE_STATUS_INITIAL 0
#define DIAE_DEVICE_STATUS_IN_LIST 1
#define DIAE_DEVICE_STATUS_JUST_ADDED 2

#define DIAE_DEVICE_NOERROR 0
#define DIAE_DEVICE_THREAD_ERROR 1
#define DIAE_DEVICE_NOATTEMPTS_ERROR 2
#define DIAE_DEVICE_CANT_START_DRIVER 3


class DiaDevice {
public:
    char _Buf[DIAE_DEVICE_BUFFER_SIZE];
    int _Bytes_Read;
	int NeedWorking;
    int _handler;
    int ToBeDeleted;
    char * _PortName;
    int _CheckStatus;
    int DeviceType;
    void * Manager;

    int Open();
    DiaDevice(char *portName);
    ~DiaDevice();
    int ReadPortBytes() {
        _Buf[0]=0;
        _Bytes_Read = 0;
        //printf("reading from handler %d\n", _handler);
        int n = read(_handler, _Buf, sizeof(_Buf)-1);
        if(n <= 0) {
            //printf("error reading serial %s\n", _PortName);
            return 0;
        }
        else {
            _Bytes_Read = n;
            _Buf[n] = 0;
            return n;
        }
    }
};

int DiaDevice_OpenPort(DiaDevice * device, int speedCode);
int DiaDevice_CloseDevice(DiaDevice * device);
int DiaDevice_ReadPortBytes(DiaDevice * device);
int DiaDevice_WritePort(DiaDevice * device, const char * buf, size_t len);

#endif
