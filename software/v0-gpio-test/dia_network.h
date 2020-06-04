#ifndef _DIA_NETWORK_H
#define _DIA_NETWORK_H

#define DIA_NETWORK_MAX_REQUEST 8192

#define DIA_ERR_HOST_NOT_FOUND 1
#define DIA_ERR_SOCKET_NOT_OPENED 2
#define DIA_ERR_NULL_REQUEST 3
#define DIA_ERR_NULL_REQUEST_FUNCTION 4
#define APP_HOST "app.diae.ru"

#include <string>
#include "dia_channel.h"

class DiaRequest;

class DiaNetwork {
public:
    pthread_t WorkingThread;
    int ToBeDestroyed;
    DiaChannel<DiaRequest> * RequestsChannel; //10 request in queue max
    DiaNetwork();
    ~DiaNetwork();
};

void * DiaNetwork_ThreadWorker(void * DiaNetworkObject);

class DiaRequest {
    public:
    DiaNetwork * Network;
    const char * Host;
    int Port;

    char Response[DIA_NETWORK_MAX_REQUEST];
    int ResponseSize;
    char RequestBuffer[DIA_NETWORK_MAX_REQUEST];
    DiaRequest(DiaNetwork * network, const char * host, int port);
};


int DiaNetwork_SendRequest(DiaNetwork * network, DiaRequest * request, char * response, int response_size);
#endif
