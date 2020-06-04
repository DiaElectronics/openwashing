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

enum bill {rub_10 = 2, rub_50 = 3, rub_100 = 4, rub_500 = 5, rub_1000 = 6, rub_5000 = 7};

DiaCcnet::DiaCcnet(DiaDevice * device, void (*incomingMoneyHandler)(void * nv9, int moneyType, int newMoney) ){
    _Device = device;
    IncomingMoneyHandler = NULL;
    DiaDevice_SetDriver(device, 0, this);
    this->IncomingMoneyHandler = incomingMoneyHandler;
}

int DiaCcnet::SendCommand(uint8_t *command) {
    uint8_t total_length = command[2];
    return DiaDevice_WritePort(_Device, (const char *)command, total_length);
}

int DiaCcnet::IsCcnet() {
    printf("is_ccnet\n");
    const int response_size = 1024;
    uint8_t code = -1;
    SendCommand(ccnet_poll);
    usleep(100*1000);
    int returned_bytes = DiaDevice_ReadPortBytes(_Device);
    printf("ccnet returned %d bytes\n", returned_bytes);
    return returned_bytes > 3;
}

int DiaCcnet::GetCode(uint8_t * buffer, int bytes_read) {
    if (bytes_read<4) {
        return -1;
    }
    return buffer[3];
}
int DiaCcnet::SendCcnetAndReadAnswerCode(uint8_t* buffer) {
    assert(buffer);
    SendCommand(buffer);
    usleep(100000);
    int bytes_read = DiaDevice_ReadPortBytes(_Device);
    int code = GetCode((uint8_t*)_Device->_Buf, bytes_read);
    printf("code=%d\n", code);
    if (code<0) {
        return 1;
    }
    return 0;
}

int DiaCcnet::GetBanknoteCode() {
    return this->_Device->_Buf[4];
}

void * DiaCcnet_Thread(void * args) {
    assert(args);
    DiaCcnet * ccnet_device = (DiaCcnet *)args;
    while(ccnet_device->_Device->NeedWorking) {
        int code = 0;
        if (code = ccnet_device->SendCcnetAndReadAnswerCode(ccnet_poll)) {
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
            case rub_500:
                new_money = 500;
                break;
            case rub_1000:
                new_money = 1000;
                break;
            case rub_5000:
                new_money = 5000;
                break;
            default:
            printf("unknown bancknote %d\n", banknote);
                break;
            }
            ccnet_device->SendCommand(ccnet_ack);
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

int DiaCcnet::Run() {
    StartDevice();
    return pthread_create(&MainThread, NULL, DiaCcnet_Thread, this);
}

DiaCcnet::~DiaCcnet() {
    printf("Destroying ccnet\n");
}

int DiaCcnet::StartDevice() {
    // RESET
    printf("reset>>\n");
    if (SendCcnetAndReadAnswerCode(ccnet_reset)<0) return 10;

    printf("poll>>\n");
    if (SendCcnetAndReadAnswerCode(ccnet_poll)<0) return 20;

    printf("identif>>\n");
    if (SendCcnetAndReadAnswerCode(ccnet_identif)<0) return 30;

    printf("poll>>\n");
    if (SendCcnetAndReadAnswerCode(ccnet_poll)<0) return 40;

    printf("enable>>\n");
    if (SendCcnetAndReadAnswerCode(ccnet_enable)<0) return 50;

    return 0;
}
