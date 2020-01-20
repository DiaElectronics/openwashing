#include "dia_device.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>


DiaDevice::DiaDevice(char *portName) {
    _Buf[0] = 0;
    NeedWorking = 1;
    _PortName = strdup(portName);
}

DiaDevice::~DiaDevice()
{
    if(_PortName!=NULL) {
        free(_PortName);
    }
}

int DiaDevice_SetPortBlocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
            printf ("error %d from tggetattr", errno);
            return 1;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
            printf("error %d setting term attributes", errno);
            return 1;
        }
        return 0;
}

int DiaDevice::Open() {
    int err = DiaDevice_OpenPort(this, B9600);
    if(err!=0) {
        this->ToBeDeleted = 1;
        printf("can't open port:%s\n", this->_PortName);
        return 1;
    }
    return 0;
}

int DiaDevice_CloseDevice(DiaDevice * device)
{
    if(device == NULL) return 1;

    close(device->_handler);
    free(device);
    return 0;
}

int DiaDevice_ReadPortBytes(DiaDevice * device)
{
    device->_Buf[0]=0;
    int n = read(device->_handler, device->_Buf, sizeof(device->_Buf)-1);
    if(n <= 0) {
        return -1;
    }
    else {
        device->_Bytes_Read = n;
        device->_Buf[n] = 0;
        return n;
    }
}

void flushSerial(int serial_port) {
    tcflush( serial_port, TCIFLUSH);
}

int openSerial(const char * serial_name, int& serial_port) {
   serial_port = open(serial_name, O_RDWR|O_NDELAY);

   struct termios tty;
   memset(&tty, 0, sizeof tty);

   cfsetospeed (&tty, (speed_t)B9600);
   cfsetispeed (&tty, (speed_t)B9600);
   tty.c_cflag     &=  ~PARENB;
   tty.c_cflag     &=  ~CSTOPB;
   tty.c_cflag     &=  ~CSIZE;
   tty.c_cflag     |=  CS8;
   tty.c_cflag     &=  ~CRTSCTS;
   tty.c_cc[VMIN]   =  0;
   tty.c_cc[VTIME]  =  5;
   tty.c_cflag     |=  CREAD | CLOCAL;

   cfmakeraw(&tty);
   flushSerial(serial_port);

   if ( tcsetattr ( serial_port, TCSANOW, &tty ) != 0)
   {
       printf("Error %d from tcsetattr\n", errno);
       return false;
   }
   return true;
}

int DiaDevice_OpenPort(DiaDevice * device, int speedCode) {
    if(device == NULL || device->_PortName == NULL) return 1;
    delay(500);
    int hnd;
    openSerial(device->_PortName, hnd);
    device->_handler = hnd;
    return DIAE_DEVICE_NOERROR;
}

int DiaDevice_WritePort(DiaDevice * device, const char * buf, size_t len) {
    if(device == NULL)
    {
        perror("NULL device\n");
        return -1;
    }
    size_t res = write(device->_handler, buf, len);
    if(res<0)
    {
        printf("can't write\n");
        return -1;
    }
    if(res<len)
    {
        printf("written less than expected %zu < %zu\n", res, len);
        return -1;
    }
    return 0;
}
