#include "dia_ccnet.h"
#include "stdio.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "dia_device.h"
#include <assert.h>

uint8_t ccnet_test_cmd[] = {0x02, 0x03, 0x06, 0x20, 0xDA, 0x81};
uint8_t ccnet_poll[] = {0x02, 0x03, 0x06, 0x33, 0xDA, 0x81};
uint8_t ccnet_ack[] = {0x02, 0x03, 0x06, 0x00, 0xC2, 0x82};
uint8_t ccnet_reset[] = {0x02, 0x03, 0x06, 0x30, 0x41, 0xB3};
uint8_t ccnet_identif[] = {0x02, 0x03, 0x06, 0x37, 0xFE, 0xC7};
uint8_t ccnet_stack[] = {0x02, 0x03, 0x06, 0x35, 0xEC, 0xE4};
uint8_t ccnet_enable[] = {0x02, 0x03, 0x0C, 0x34, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xB5, 0xC1};

enum responseCode {PowerUp = 0x10, Initialize = 0x13, Idling = 0x14, Accepting = 0x15, Stacking = 0x17, Returning = 0x18,
			Disabled = 0x19, Holding = 0x1A, Busy = 0x1B, Rejecting = 0x1C, Dispensing = 0x1D, Unloading = 0x1E,
			SettingTypeCassette = 0x21, Dispensed = 0x25, Unloaded = 0x26, InvalidBillNumber = 0x28,
			SetCassetteType = 0x29, IvalidCommand = 0x30, DropCassetteFull = 0x41, DropCassetteRemoved = 0x42,
			JamInAcceptor = 0x43, JamInStacker = 0x44, Cheated = 0x45, Error = 0x47, Escrow = 0x80, Stacked = 0x81,
			Returned = 0x82};

enum bill {rub_10 = 2, rub_50 = 3, rub_100 = 4, rub_500 = 5, rub_1000 = 6, rub_5000 = 7, rub_200=12, rub_2000=13};

DiaCcnet::DiaCcnet(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) ){
    _Device = device;
    IncomingMoneyHandler = NULL;
    //DiaDevice_SetDriver(device, 0, this);
    this->IncomingMoneyHandler = incomingMoneyHandler;
}

int DiaCcnet_SendCommand(DiaDevice * device, uint8_t *command) {
    uint8_t total_length = command[2];
    return DiaDevice_WritePort(device, (const char *)command, total_length);
}

int DiaCcnet_Detect(DiaDevice * device) {
    printf("is_ccnet\n");
    DiaCcnet_SendCommand(device, ccnet_poll);
    usleep(100*1000);
    int returned_bytes = DiaDevice_ReadPortBytes(device);
    printf("ccnet returned %d bytes\n", returned_bytes);
    return returned_bytes > 3;
}

int DiaCcnet_GetCode(uint8_t * buffer, int bytes_read) {
    if (bytes_read<4) {
        return -1;
    }
    return buffer[3];
}
int DiaCcnet_SendCcnetAndReadAnswerCode(DiaDevice * device, uint8_t* buffer) {
    assert(buffer);
    DiaCcnet_SendCommand(device, buffer);
    usleep(100000);
    int bytes_read = DiaDevice_ReadPortBytes(device);
    int code = DiaCcnet_GetCode((uint8_t*)device->_Buf, bytes_read);
    printf("code=%d\n", code);
    return code;
}

int DiaCcnet::GetBanknoteCode() {
    return this->_Device->_Buf[4];
}

void * DiaCcnet_Thread(void * args) {
    assert(args);
    DiaCcnet * ccnet_device = (DiaCcnet *)args;
    while(ccnet_device->_Device->NeedWorking) {
        int code = DiaCcnet_SendCcnetAndReadAnswerCode(ccnet_device->_Device, ccnet_poll);
        if (code<0) {
            printf("error sending to ccnet_device");
        }
        if (code == Stacked) {
            int banknote = ccnet_device->GetBanknoteCode();
            int new_money = 0;
            switch (banknote)
            {
            case rub_10:
                new_money = 10;
                break;
            case rub_50:
                new_money = 50;
                break;
            case rub_100:
                new_money = 100;
                break;
            case rub_200:
                new_money = 200;
                break;
            case rub_500:
                new_money = 500;
                break;
            case rub_1000:
                new_money = 1000;
                break;
            case rub_2000:
                new_money = 2000;
                break;
            case rub_5000:
                new_money = 5000;
                break;
            default:
                new_money=banknote;
            printf("unknown bancknote %d\n", banknote);
                break;
            }
            DiaCcnet_SendCommand(ccnet_device->_Device, ccnet_ack);
            if(new_money >0) {
                if (ccnet_device->IncomingMoneyHandler){
                    ccnet_device->IncomingMoneyHandler(ccnet_device->_Device->Manager, 0, new_money);
                } else {
                    printf("Can't report money :( \n");
                }
            }
        }
        usleep(500*1000);
    }
    pthread_exit(NULL);
    return 0;
}

int DiaCcnet_StartDriver(DiaCcnet * banknoteAcceptor) {
    banknoteAcceptor->StartDevice();
    return pthread_create(&banknoteAcceptor->MainThread, NULL, DiaCcnet_Thread, banknoteAcceptor);
}

DiaCcnet::~DiaCcnet() {
    printf("Destroying ccnet\n");
}

int DiaCcnet::StartDevice() {
    // RESET
    printf("reset>>\n");
    if (DiaCcnet_SendCcnetAndReadAnswerCode(this->_Device, ccnet_reset)<0) return 10;

    printf("poll>>\n");
    if (DiaCcnet_SendCcnetAndReadAnswerCode(this->_Device, ccnet_poll)<0) return 20;

    printf("identif>>\n");
    if (DiaCcnet_SendCcnetAndReadAnswerCode(this->_Device, ccnet_identif)<0) return 30;

    printf("poll>>\n");
    if (DiaCcnet_SendCcnetAndReadAnswerCode(this->_Device, ccnet_poll)<0) return 40;

    printf("enable>>\n");
    if (DiaCcnet_SendCcnetAndReadAnswerCode(this->_Device, ccnet_enable)<0) return 50;

    return 0;
}
