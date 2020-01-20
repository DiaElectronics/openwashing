#ifndef _DIA_STORAGE_H
#define _DIA_STORAGE_H

#define STORAGE_NOERROR 0
#define STORAGE_GENERAL_ERROR 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SimpleRedisClient.h"
#include "dia_functions.h"
#include "dia_storage_interface.h"

int DiaStore_Load(void * object, const char *key, void *data, size_t max_length);
int DiaStore_Save(void * object, const char *key, const void *data, size_t length);

class DiaStore{
public:
    SimpleRedisClient rc;

    int Save(const char * key, const void *data, size_t length) {
        char cmd[2048];
        char data_buffer[2048];
        base64_encode((const unsigned char *)data, length, data_buffer, sizeof(data_buffer));
        int key_length = strlen(key);
        sprintf(cmd, "%s", key);
        cmd[key_length]=' ';
        sprintf(&cmd[key_length+1], "%s", data_buffer);
        rc = cmd;
        if(rc[key]) {
            return STORAGE_NOERROR;
        }
        printf("error saving value\n");
        return STORAGE_GENERAL_ERROR;
    }

    int Load(const char * key, void *data, size_t length) {
        if(rc[key]) {
            printf("extracted value is [%s]\n", (char*)rc);
            int err = base64_decode((char*)rc, strlen((char*)rc), (char *)data, length);
            if (err<0) {
                return STORAGE_GENERAL_ERROR;
            }
            return STORAGE_NOERROR;
        }
        printf("key is not found \n");
        return STORAGE_GENERAL_ERROR;
    }

    int IsOk() {
        if(!rc)
        {
            printf("Redis connection failed\n");
            return 0;
        }
        printf("Redis connection established\n");
        return 1;
    }

    int Init() {
        rc.setHost("127.0.0.1");
        rc.LogLevel(0);
        rc.auth("diae_password_14#15_horse_is_Very_fast");
        int isOk = IsOk();
        return isOk;
    }


    DiaStore() {

    }

    ~DiaStore() {
        rc.redis_close();
    }

    storage_interface_t * CreateInterface() {
        storage_interface_t * result = (storage_interface_t *)calloc(1, sizeof(storage_interface_t));
        result->object = this;
        result->load = DiaStore_Load;
        result->save = DiaStore_Save;
        result->next_object = 0;
        result->is_real = 1;
        return result;
    }
};

#endif
