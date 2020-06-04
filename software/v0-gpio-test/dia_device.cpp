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
    IncomingDataHandler = NULL;
    IncomingDataObject = NULL;
}

DiaDevice::~DiaDevice()
{
    if(_PortName!=NULL) {
        free(_PortName);
    }
    if(_DeviceDataEvent!=NULL) {
		event_free(_DeviceDataEvent);
	}
	if(_DeviceEventBase!=NULL) {
  		event_base_free(_DeviceEventBase);
	}
}

void DiaDevice_SetDriver(DiaDevice *device, void (*IncomingDataHandler)(void * specificDriver, int buffer_size, char * buffer), void  * incomingDataObject)
{
    device->IncomingDataHandler = IncomingDataHandler; //DiaNv9Usb_CommandReadingThread
    device->IncomingDataObject = incomingDataObject; //NV 9 Driver
}

void DiaDevice_DataReceivedHandler(int handler, short int y, void *pargs)
{
    char buf[256];
    DiaDevice * device = (DiaDevice *)pargs;
    int bytes_read = read(handler, buf, sizeof(buf)-1);

    if(bytes_read>0)
    {
        buf[bytes_read]=0;
        printf("read %d bytes:" , bytes_read);
        for(int i=0;i<bytes_read;i++)
        {
			printf("%d ", buf[i]);
		}
		printf("\n");
        if(device->IncomingDataHandler!=NULL)
        {
			printf("forwarding data...\n");
            device->IncomingDataHandler(device->IncomingDataObject, bytes_read, buf);
        }
        else
        {
			printf("data handler is null\n");
		}
    }
    if(bytes_read <0)
    {
        //printf(" %s error happened while recieved data from device\n", device->_PortName);
        //perror(strerror(errno));
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

//MAIN PROCEDURE
void * DiaDevice_CommunicationThread(void * devicePtr)
{
    DiaDevice * device = (DiaDevice*) devicePtr;

    while(device->NeedWorking)
    {
		DiaDevice_DataReceivedHandler(device->_handler, 0, device);
		delay(10);
	}

    //device->_DeviceEventBase = event_init();

    //device->_DeviceDataEvent = event_new(device->_DeviceEventBase,
    //device->_handler, EV_READ | EV_PERSIST, DiaDevice_DataReceivedHandler, device );
    //event_add(device->_DeviceDataEvent, NULL);

    //event_base_dispatch(device->_DeviceEventBase);
    //please add code to check the health of the device now

    pthread_exit(NULL);
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

int DiaDevice_StartDeviceThread(DiaDevice * device)
{
    int err = pthread_create(&(device->DeviceCommunicationThread), NULL, &DiaDevice_CommunicationThread, (void *)device);
    if (err != 0)
    {
        printf("\ncan't create thread :[%s]", strerror(err));
        return DIAE_DEVICE_THREAD_ERROR;
    }
    return DIAE_DEVICE_NOERROR;
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

int DiaDevice_OpenPort(DiaDevice * device, int speedCode)
{
    if(device == NULL || device->_PortName == NULL) return 1;

    int fd; /* File descriptor for the port */
    delay(500);
    fd = open(device->_PortName, O_RDWR | O_NOCTTY | O_NDELAY );

    int count = 0;
    int res = ioctl(fd, 0x54FF, &count);

    if(count>1)
    {
        printf("port is already in use");
        return 1;
    }

    if (fd == -1)
    {
        perror("open_port: Unable to open port: ");
        device->ToBeDeleted = 1;
        return 1;
    }
    else
    {
        int parity = 0;
        struct termios options;
        tcgetattr(fd, &options);
        cfsetospeed(&options, speedCode);
        cfsetispeed(&options, speedCode);
        DiaDevice_SetPortBlocking(fd, 0);
        /* set raw input, 1 second timeout */
        options.c_cflag     |= (CLOCAL | CREAD);
        //options.c_lflag     &= ~(ICANON | ISIG);
        //options.c_oflag     &= ~OPOST;
        //additional options
        options.c_oflag =0;
        options.c_iflag &= ~IGNBRK;
        options.c_lflag = 0;
        options.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
        //options.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        options.c_cflag |= parity;
        //options.c_cflag &= ~CSTOPB;
        //options.c_cflag &= ~CRTSCTS;
        //end additional options
        options.c_cc[VMIN]  = 0;
        options.c_cc[VTIME] = 10;

        /* set the options */
        tcsetattr(fd, TCSANOW, &options);

        fcntl(fd, F_SETFL, FNDELAY);
    }
    device->_handler = fd;
    return DIAE_DEVICE_NOERROR;
}

int DiaDevice_WritePort(DiaDevice * device, const char * buf, size_t len)
{
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
