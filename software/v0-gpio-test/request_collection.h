#ifndef _DIA_REQUEST_COLLECTION
#define _DIA_REQUEST_COLLECTION

#include "dia_network.h"

int DiaPowerOnRequest_Send(DiaNetwork * network, const char * firmwareVersion, const char * statusText, const char * deviceName);
int DiaStatusRequest_Send(DiaNetwork * network, const char * firmwareVersion, const char * deviceName,
    double totalIncomeCoins, double totalIncomeBanknotes, double totalIncomeElectron, double totalIncomeService);
#endif

