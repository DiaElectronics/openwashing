#include <stdio.h>
#include <wiringPi.h>

#include "dia_gpio.h"
#include "dia_screen.h"
#include "dia_devicemanager.h"
#include "dia_network.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "dia_security.h"
#include "dia_functions.h"
#include "dia_configuration.h"
#include "dia_runtime.h"
#include "dia_runtime_registry.h"
#include <string>
#include <map>
#include <time.h>
#include <unistd.h>

#define IDLE 1
#define WORK 2
#define PAUSE 3

#define DIA_VERSION "v1.4-enlight"

//#define USE_GPIO
#define USE_KEYBOARD

#define CENTRALWASH_KEY "/home/pi/centralwash.key"
#define BILLION 1000000000

DiaConfiguration * config;

int _100MsIntervalsCount;
int _100MsIntervalsCountRelay;

// Public key for signing every request to Central Server.
const int centralKeySize = 6;
std::string centralKey;

int _DebugKey = 0;

// Variable for storing an additional money.
// For instance, service money from Central Server can be transfered inside.
int _Balance = 0;

int GetKey(DiaGpio * _gpio) {
    int key = 0;

#ifdef USE_GPIO
    key = DiaGpio_GetLastKey(_gpio);
#endif

#ifdef USE_KEYBOARD
    if (_DebugKey != 0) {
        key = _DebugKey;
    }
    _DebugKey = 0;
#endif

    if (key) {
        printf("Key %d reported\n", key);
    }
    return key;
}

// Main object for Client-Server communication.
DiaNetwork network;

// Saves new income money and creates money report to Central Server.
void SaveIncome() {
    if (config && config->GetStorage()) {
        storage_interface_t * storage = config->GetStorage();
        storage->save(storage->object, "income", &(config->_Income), sizeof(income));

        network.SendMoneyReport((int)config->_Income.carsTotal,
        (int)config->_Income.totalIncomeCoins,
        (int)config->_Income.totalIncomeBanknotes,
        (int)config->_Income.totalIncomeElectron,
        (int)config->_Income.totalIncomeService);
    }
}

////// Runtime functions ///////
int get_key(void *object) {
    return GetKey((DiaGpio *)object);
}

int turn_light(void *object, int pin, int animation_id) {
    #ifdef USE_GPIO
    DiaGpio * gpio = (DiaGpio *)object;
    gpio->AnimationSubCode = pin;
    gpio->AnimationCode = animation_id;
    #endif
    return 0;
}

// Creates receipt request to Online Cash Register.
int send_receipt(int postPosition, int isCard, int amount) {
    return network.ReceiptRequest(postPosition, isCard, amount);
}

int turn_program(void *object, int program) {
    #ifdef USE_GPIO
    DiaGpio * gpio = (DiaGpio *)object;
    // negative number will stop all
    if (program >= MAX_PROGRAMS_COUNT) {
        return 1;
    }
    gpio->CurrentProgram = program;
    #endif
    return 0;
}

int get_coins(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->CoinMoney + _Balance;

    manager->CoinMoney  = 0;
    _Balance = 0;
        
    if (curMoney > 0) {
        printf("coin %d\n", curMoney);

        if (config) {
            config->_Income.totalIncomeCoins += curMoney;
            SaveIncome();
        }
    }
    return curMoney;
}

int get_banknotes(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->BanknoteMoney;
    if (curMoney > 0) {
        printf("bank %d\n", curMoney);
        if (config) {
            config->_Income.totalIncomeBanknotes += curMoney;
            SaveIncome();
        }
        manager->BanknoteMoney  = 0;
        _Balance = 0;
    }
    return curMoney;
}

int get_electronical(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->ElectronMoney;
    if (curMoney>0) {
        printf("electron %d\n", curMoney);
        if (config) {
            config->_Income.totalIncomeElectron += curMoney;
            SaveIncome();
        }
        manager->ElectronMoney  = 0;
        _Balance = 0;
    }
    return curMoney;
}

// Tries to perform a bank card NFC transaction.
// Gets money amount.
int request_transaction(void *object, int money) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    if (money > 0) {
        DiaDeviceManager_PerformTransaction(manager, money);
        return 0;
    }
    return 1;
}

// Returns a status of NFC transaction. 
// Returned value == amount of money, which are expected by the reader.
// For example, 0 - reader is offline; 100 - reader expects 100 RUB.
int get_transaction_status(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int status = DiaDeviceManager_GetTransactionStatus(manager);
    fprintf(stderr, "Transaction status: %d\n", status);
    return status;
}

// Deletes actual NFC transaction. 
int abort_transaction(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    DiaDeviceManager_AbortTransaction(manager);
    return 0;
}

int smart_delay_function(void * arg, int ms) {
    struct timespec current_time;
    struct timespec *stored_time = (struct timespec *) arg;

    uint64_t delay_real = 1000*ms;
    uint64_t delay_wanted = delay_real;

    clock_gettime(CLOCK_MONOTONIC_RAW, &current_time);

    //us (usecond) = 10^-6 seconds
    uint64_t delta_us = (current_time.tv_sec - stored_time->tv_sec) * 1000000 + (current_time.tv_nsec - stored_time->tv_nsec) / 1000;

    if (delta_us<delay_wanted) {
        delay_real=delay_wanted - delta_us;
    } else {
        stored_time->tv_nsec = current_time.tv_nsec;
        stored_time->tv_sec = current_time.tv_sec;
    }
    usleep(delay_real);
    stored_time->tv_nsec = stored_time->tv_nsec + delay_wanted * 1000;
    if (stored_time->tv_nsec>=BILLION) {
        stored_time->tv_nsec = stored_time->tv_nsec - BILLION;
        stored_time->tv_sec +=1;
    }
    return 0;
}
/////// End of Runtime functions ///////


/////// Central server communication functions //////

// Sends PING request to Central Server every 2 seconds.
// May get service money from server.
int CentralServerDialog() {
    
    if (!config || !config->GetGpio()) {
        return 0;
    }
    
    _100MsIntervalsCount++;
    if(_100MsIntervalsCount < 0) {
        printf("Memory corruption on _100MsIntervalsCount\n");
        _100MsIntervalsCount = 0;
    }

    _100MsIntervalsCountRelay++;
    if(_100MsIntervalsCountRelay < 0) {
        printf("Memory corruption on _100MsIntervalsCountRelay\n");
        _100MsIntervalsCountRelay = 0;
    }

    // Every 2 seconds we go inside this
    if (_100MsIntervalsCount > 20) {
        _100MsIntervalsCount = 0;
        
        printf("Sending another PING request to server...\n");

        int serviceMoney = 0;
        network.SendPingRequest(network.GetHostName(), serviceMoney);

        
        if (serviceMoney > 0) {
	    _Balance += serviceMoney;
            config->_Income.totalIncomeService += serviceMoney;
            SaveIncome();
        } 
           
    }

    // Every 5 min (300 sec) we go inside this
    if (_100MsIntervalsCountRelay > 3000) {
        _100MsIntervalsCountRelay = 0;
        
        printf("Sending relay report to server...\n");
        
        RelayStat *relays = new RelayStat[MAX_RELAY_NUM];

        DiaGpio * gpio = config->GetGpio();

        for (int i = 0; i < MAX_RELAY_NUM; i++) {
            relays[i].switched_count = gpio->Stat.relay_switch[i+1];
            relays[i].total_time_on = gpio->Stat.relay_time[i+1];
        }
        
        network.SendRelayReport(relays);

        delete relays;
        
    }
    return 0;
}

int RecoverMoney() {
    money_report_t* last_money_report = new money_report_t;

    int err = network.GetLastMoneyReport(last_money_report);

    if (err == 0) 
    {
        fprintf(stderr,"CarsTotal:%d CoinsTotal:%d BanknotesTotal:%d CashlessTotal:%d TestTotal:%d\n",
        last_money_report->cars_total,
        last_money_report->coins_total,
        last_money_report->banknotes_total,
        last_money_report->cashless_total,
        last_money_report->service_total);

        // Update the local storage if needed
        if ((config->_Income.totalIncomeCoins < last_money_report->coins_total) ||
        (config->_Income.totalIncomeBanknotes < last_money_report->banknotes_total) ||
        (config->_Income.totalIncomeElectron < last_money_report->cashless_total) ||
        (config->_Income.totalIncomeService < last_money_report->service_total) ||
        (config->_Income.carsTotal < last_money_report->cars_total)) 
        {
	    printf("Money in config updated\n");
            config->_Income.totalIncomeCoins = last_money_report->coins_total;
            config->_Income.totalIncomeBanknotes = last_money_report->banknotes_total;
            config->_Income.totalIncomeElectron = last_money_report->cashless_total;
            config->_Income.totalIncomeService = last_money_report->service_total;
            config->_Income.carsTotal = last_money_report->cars_total;
            SaveIncome();
        }
    } 
    else 
    {
        fprintf(stderr,"Get last money report error: %d\n",err);
    }

    delete last_money_report;
    return err;
}

int RecoverRelay() {
    relay_report_t* last_relay_report = new relay_report_t;

    int err = network.GetLastRelayReport(last_relay_report);

    if (err == 0) {
        DiaGpio * gpio = config->GetGpio();
	for (int i = 0; i < MAX_RELAY_NUM; i++) {
		printf("Relay %d: switched - %d, total - %d\n", i, last_relay_report->RelayStats[i].switched_count,
		last_relay_report->RelayStats[i].total_time_on*1000);
	}

        bool update = false;
        for(int i = 0; i < MAX_RELAY_NUM; i++) {
            if ((gpio->Stat.relay_switch[i+1] < last_relay_report->RelayStats[i].switched_count) ||
                (gpio->Stat.relay_time[i+1] < last_relay_report->RelayStats[i].total_time_on*1000)) {
                    update = true;
            }
        }
        if (update) {
            fprintf(stderr,"Relays in config updated\n");
            for(int i = 0; i < MAX_RELAY_NUM; i++) {
                gpio->Stat.relay_switch[i+1] = last_relay_report->RelayStats[i].switched_count;
                gpio->Stat.relay_time[i+1] = last_relay_report->RelayStats[i].total_time_on*1000;
            }
        }
    } else {
            fprintf(stderr,"Get last relay report err:%d\n",err);
    }

    delete last_relay_report;
    return err;
}
//////// End of Central server communication functions /////////

//////// Local registry Save/Load functions /////////
std::string GetLocalData(std::string key) {
    std::string filename = "registry_" + key + ".reg";
    if (file_exists(filename.c_str())) {
        char value[2];
        dia_security_read_file(filename.c_str(), value, sizeof(2));
        
        return std::string(value, 2);
    }
    else {
        return "0";
    }
}

void SetLocalData(std::string key, std::string value) {
    std::string filename = "registry_" + key + ".reg";
    fprintf(stderr, "Key: %s, Value: %s \n", key.c_str(), value.c_str());
    dia_security_write_file(filename.c_str(), value.c_str());
}
/////////////////////////////////////////////////////

/////// Registry processing function ///////////////
int RecoverRegistry() {
    printf("Recovering registries...\n");

    DiaRuntimeRegistry* registry = &(config->GetRuntime()->Registry);

    std::string value = "";

    int tmp = 0;
    int err = network.SendPingRequest(network.GetHostName(), tmp);
    std::string default_price = "15";

    if (!err) {
        // Load all prices in online mode
        fprintf(stderr, "Online mode, registry got from Central Server...\n");

        for (int i = 1; i < 7; i++) {
            std::string key = "price" + std::to_string(i);
            std::string value = network.GetRegistryValueByKey(key);

            if (value != "") {
                fprintf(stderr, "Key-value read online => %s:%s; \n", key.c_str(), value.c_str());
            } else {
		fprintf(stderr, "Server returned empty value, setting default...\n");
		value = default_price;
		network.SetRegistryValueByKey(key, value);
	    }
	    registry->SetValue(key.c_str(), value.c_str());

	    std::string localData = GetLocalData(key);
	    if (localData != value) {
		SetLocalData(key, value);
	    }
        }
    } else {
        fprintf(stderr, "Offline mode, checking local registries...\n");

        // 6 washing modes ~ 6 prices
        for (int i = 1; i < 7; i++) {
            std::string current_key = "price" + std::to_string(i);
            
            std::string value = GetLocalData(current_key);
            if (value == "0") {
                value = default_price;
            } 
            
            registry->SetValue(current_key.c_str(), value.c_str());
            fprintf(stderr, "Key-value read locally => %s:%s; \n", current_key.c_str(), value.c_str());
        }
    }  
    return err;
}
//////////////////////////////////////////////

// Just compilation of recovers.
void RecoverData() {
    RecoverRegistry();
    RecoverMoney();
    RecoverRelay();
}

int main(int argc, char ** argv) {
    config = 0;

    // Timer initialization
    struct timespec stored_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &stored_time);

    if (argc > 2) {
        fprintf(stderr, "Too many parameters. Please leave just folder with the firmware, like [firmware.exe .] \n");
        return 1;
    }

    // Set working folder 
    std::string folder = "./firmware";
    if (argc == 2) {
        folder = argv[1];
    }
    printf("Looking for firmware in [%s]\n", folder.c_str());
    printf("Version: %s\n", DIA_VERSION);

    // Check public key on disk
    // If it doesn't exist - get it from MAC
    if (file_exists(CENTRALWASH_KEY)) {
        // Size is multiplied by 2, because of Hex numbers in file with width == 2 
        char rawKey[centralKeySize * 2 + 1];

        dia_security_read_file(CENTRALWASH_KEY, rawKey, centralKeySize * 2 + 1);
        centralKey = std::string(rawKey);
        printf("Public key read from file: %s \n", centralKey.c_str());

    } else {
        centralKey = network.GetMacAddress(centralKeySize);
        printf("MAC address: %s\n", centralKey.c_str());

        dia_security_write_file(CENTRALWASH_KEY, centralKey.c_str());
        printf("Public key wrote to file: %s \n", CENTRALWASH_KEY);
    }
    
    network.SetPublicKey(std::string(centralKey));
    int need_to_find = 1; 
    std::string serverIP = "";

    while (need_to_find) {
    	serverIP = network.GetCentralServerAddress();
    	if (serverIP == "") {
        	printf("Error: Center Server is unavailable. Next try...\n");
    	} else
	    need_to_find = 0;
    }
    network.SetHostAddress(serverIP);

    // Runtime and firmware initialization
    DiaDeviceManager *manager = new DiaDeviceManager;
    DiaDeviceManager_AddCardReader(manager);

    SDL_Event event;
    
    
    DiaConfiguration configuration(folder);
    config = &configuration;
    int err = configuration.Init();
    if (err != 0) {
        printf("Can't run due to the configuration error\n");
        return 1;
    }
    
    // Get working data from server: money, relays, prices
    RecoverData();

    printf("Configuration is loaded...\n");

    // Screen load
    
    std::map<std::string, DiaScreenConfig *>::iterator it;
    for (it = configuration.ScreenConfigs.begin(); it != configuration.ScreenConfigs.end(); it++) {
        std::string currentID = it->second->id;
        DiaRuntimeScreen * screen = new DiaRuntimeScreen();
        screen->Name = currentID;
        screen->object = (void *)it->second;
        screen->set_value_function = dia_screen_config_set_value_function;
        screen->screen_object = configuration.GetScreen();
        screen->display_screen = dia_screen_display_screen;
        configuration.GetRuntime()->AddScreen(screen);
    }
    
    // Program load
    
    #ifdef USE_GPIO
    configuration.GetRuntime()->AddPrograms(&configuration.GetGpio()->_ProgramMapping);
    #else
    configuration.GetRuntime()->AddPrograms(0);
    #endif

    configuration.GetRuntime()->AddAnimations();
  
    DiaRuntimeHardware * hardware = new DiaRuntimeHardware();
    hardware->keys_object = configuration.GetGpio();
    hardware->get_keys_function = get_key;

    hardware->light_object = configuration.GetGpio();
    hardware->turn_light_function = turn_light;

    hardware->program_object = configuration.GetGpio();
    hardware->turn_program_function = turn_program;

    hardware->send_receipt_function = send_receipt;

    hardware->coin_object = manager;
    hardware->get_coins_function = get_coins;

    hardware->banknote_object = manager;
    hardware->get_banknotes_function = get_banknotes;

    hardware->electronical_object = manager;
    hardware->get_electronical_function = get_electronical;    
    hardware->request_transaction_function = request_transaction;  
    hardware->get_transaction_status_function = get_transaction_status;
    hardware->abort_transaction_function = abort_transaction;

    hardware->delay_object = &stored_time;
    hardware->smart_delay_function = smart_delay_function;

    configuration.GetRuntime()->AddHardware(hardware);
    configuration.GetRuntime()->AddRegistry(&(config->GetRuntime()->Registry));
    
    // Runtime start
    int keypress = 0;

    // Call Lua setup function
    configuration.GetRuntime()->Setup();

    while(!keypress)
    {
        // Call Lua loop function
        configuration.GetRuntime()->Loop();

        // Ping server every 2 sec and probably get service money from it
        CentralServerDialog();

        // Process pressed button
        
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    keypress = 1;
                    printf("Quitting by sdl_quit\n");
                break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        
                        case SDLK_UP:
                            // Debug service money addition
                            _Balance += 10;
                            configuration._Income.totalIncomeService+=10;

                            // Save this in the storage
                            configuration.GetStorage()->save(configuration.GetStorage()->object, "income", &configuration._Income, sizeof(income));

                            printf("UP\n"); fflush(stdout);
                            break;
                        
                        case SDLK_1:
                            _DebugKey = 1;
                            printf("1\n"); fflush(stdout);
                            break;

                        case SDLK_2:
                            _DebugKey = 2;
                            printf("2\n"); fflush(stdout);
                            break;

                        case SDLK_3:
                            _DebugKey = 3;
                            printf("3\n"); fflush(stdout);
                            break;

                        case SDLK_4:
                            _DebugKey = 4;
                            printf("4\n"); fflush(stdout);
                            break;

                        case SDLK_5:
                            _DebugKey = 5;
                            printf("5\n"); fflush(stdout);
                            break;

                        case SDLK_6:
                            _DebugKey = 6;
                            printf("6\n"); fflush(stdout);
                            break;

                        case SDLK_7:
                            _DebugKey = 7;
                            printf("7\n"); fflush(stdout);
                            break;

                        default:
                            keypress = 1;
                            printf("Quitting by keypress...");
                            break;
                    }
                break;
            }
        }
    }

    delay(2000);
    return 0;
}
