#include "request_collection.h"
#include <time.h>
#include <stdio.h>

#define CUSTOMER_HOST "crm.chistobistro24.ru"


int DiaPowerOnRequest_Send(DiaNetwork * network, const char * firmwareVersion, const char * statusText, const char * deviceName) {

    DiaRequest * request = new DiaRequest(network, CUSTOMER_HOST,   80);
    unsigned long currentTime = time(NULL);
    const char * messageFormat = "%s http://%s/api_send.php?device=%s&timestamp=%lu&action=POWER_ON&details=%s,%s HTTP/1.0\r\n\r\n%s";
    sprintf(request->RequestBuffer, messageFormat, "GET", CUSTOMER_HOST, deviceName, currentTime,
    statusText, firmwareVersion,"");
    network->RequestsChannel->Push(request);
    return 0;
}

int DiaStatusRequest_Send(DiaNetwork * network, const char * firmwareVersion, const char * deviceName,
    double totalIncomeCoins, double totalIncomeBanknotes, double totalIncomeElectron, double totalIncomeService) {
    DiaRequest * request = new DiaRequest(network, CUSTOMER_HOST, 80);
    unsigned long currentTime = time(NULL);
    char moneyString[256];
    sprintf(moneyString, "0,0,%.2f,%.2f,%.2f", totalIncomeCoins, totalIncomeBanknotes, totalIncomeService);
    const char * messageFormat = "%s http://%s/api_send.php?device=%s&timestamp=%lu&action=REPORT2&details=%s,%s HTTP/1.0\r\n\r\n%s";
    sprintf(request->RequestBuffer, messageFormat, "GET", CUSTOMER_HOST, deviceName, currentTime,
    moneyString, firmwareVersion,"");
    network->RequestsChannel->Push(request);
    return 0;
}

int DiaTextRequest_Send(DiaNetwork * network, const char * deviceName,
    const char * action, const char * text) {
    DiaRequest * request = new DiaRequest(network, CUSTOMER_HOST, 80);
    unsigned long currentTime = time(NULL);


    const char * messageFormat = "%s http://%s/api_send.php?device=%s&timestamp=%lu&action=%s&details=%s HTTP/1.0\r\n\r\n";
    sprintf(request->RequestBuffer, messageFormat, "GET", CUSTOMER_HOST, deviceName, currentTime,
    action, text);
    network->RequestsChannel->Push(request);
    return 0;
}

char key_buff[1024];
const char * DiaCodeRequest_Send(DiaNetwork * network, const char * serial, const char * code) {
    DiaRequest * request = new DiaRequest(network, APP_HOST, 8000);
    const char * messageFormat = "GET http://%s:8000/v0/keys?serial=%s&code=%s HTTP/1.0\r\n\r\n";
    sprintf(request->RequestBuffer, messageFormat, APP_HOST, serial, code);
    DiaNetwork_SendRequest(network, request, key_buff, sizeof(key_buff));
    return key_buff;
}
