#include "dia_network.h"

#include <stdio.h> /* printf, sprintf */
#include <stdlib.h> /* exit */
#include <unistd.h> /* read, write, close */
#include <string.h> /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h> /* struct hostent, gethostbyname */
#include <pthread.h>

DiaRequest::DiaRequest(DiaNetwork * network, const char * host, int port) {
    Host = host;
    Port = port;
    ResponseSize = DIA_NETWORK_MAX_REQUEST;
}

DiaNetwork::DiaNetwork() {
    RequestsChannel = new DiaChannel<DiaRequest>(10);
    pthread_create(&WorkingThread, NULL, &DiaNetwork_ThreadWorker, this);

}

DiaNetwork::~DiaNetwork() {
    delete RequestsChannel;
}

// DiaNetwork_ThreadWorker is a thread to send reqeusts using a queue
void * DiaNetwork_ThreadWorker(void * DiaNetworkObject) {
    if (DiaNetworkObject == NULL) {
        printf("ERROR: NO NETWORK OBJECT PROVIDED\n");
        pthread_exit(NULL);
        return NULL;
    }
    DiaNetwork * network = (DiaNetwork *)DiaNetworkObject;
    DiaRequest **currentRequest;
    currentRequest = (DiaRequest **)malloc(sizeof(void *));
    while(!network->ToBeDestroyed) {
        network->RequestsChannel->Pop(currentRequest, true);
        DiaRequest * requestLink = *currentRequest;
        if (requestLink != NULL) {
            DiaNetwork_SendRequest(network, requestLink,0,0);
            //printf("Request:%s\n", requestLink->RequestBuffer);
            delete requestLink;
        }
    }
    free(currentRequest);
    pthread_exit(NULL);
    return NULL;
}

int DiaNetwork_SendRequest(DiaNetwork * network, DiaRequest * request, char * response, int response_size) {
    if (request == NULL) {
        return DIA_ERR_NULL_REQUEST;
    };

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        return DIA_ERR_SOCKET_NOT_OPENED;
    }
    /* lookup the ip address */
    server = gethostbyname(request->Host);
    if (server == NULL) {
        return DIA_ERR_HOST_NOT_FOUND;
    }

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(request->Port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR");
        return 3;
    }

    /* send the request */

    total = strlen(request->RequestBuffer);
    sent = 0;
    do {
        bytes = write(sockfd,request->RequestBuffer+sent,total-sent);
        if (bytes < 0) {
            break;
        }
        if (bytes == 0)
            break;
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    memset(request->Response, 0, request->ResponseSize);
    total = request->ResponseSize-1;
    received = 0;
    do {
        bytes = read(sockfd,request->Response+received,total-received);
        if (bytes < 0) {
            printf("err resp\n");
            return 0;
        }
        if (bytes == 0)
            break;
        received+=bytes;

    } while (received < total);

    if (received == total) {
        printf("ERR: %s", request->Response);
    }

    /* close the socket */
    close(sockfd);

    /* process response */
    //printf("Response:\n%s\n",request->Response);
    if(response!=0) {
        strncpy(response, request->Response, response_size );
    }
    return 0;
}
