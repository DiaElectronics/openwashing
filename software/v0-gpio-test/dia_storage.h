#ifndef _DIA_STORAGE_H
#define _DIA_STORAGE_H

#define STORAGE_NOERROR 0
#define STORAGE_GENERAL_ERROR 1

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SimpleRedisClient.h"
#include "dia_functions.h"

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

    DiaStore() {
        rc.setHost("127.0.0.1");
        rc.LogLevel(0);
        rc.auth("diae_password_14#15_horse_is_Very_fast");
        IsOk();
    }

    ~DiaStore() {
        rc.redis_close();
    }
};

#endif
