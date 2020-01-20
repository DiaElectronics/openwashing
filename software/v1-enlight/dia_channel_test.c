#include "dia_channel.h"

int main() {
    DiaChannel<int> ch;
    int err =0;
    int *res= &err;
    err = ch.Pop(&res);
    if (err != CHANNEL_BUFFER_EMPTY) {
        printf("err empty channel error is not working\n");
    }
    if (res!=0) {
        printf("err empty channel error is not working (returned not null link)\n");
    }
    int a1 = 1;
    int a2 = 2;
    int a3 = 3;
    ch.Push(&a1);
    ch.Push(&a2);
    ch.Push(&a3);
    err = ch.Pop(&res);
    if(*res!=1) {
        printf("failed 1\n");
    }
    err = ch.Pop(&res);
    if(*res!=2) {
        printf("failed 2\n");
    }
    err = ch.Pop(&res);
    if(*res!=3) {
        printf("failed 3\n");
    }
    printf("OK\n");
}
