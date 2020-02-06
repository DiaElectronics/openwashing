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

#define DIA_VERSION "v1.3-enlight"

//#define USE_GPIO
#define USE_KEYBOARD

#define CENTRALWASH_KEY "/home/pi/centralwash.key"
#define ID_KEY "/home/pi/id.txt"
#define BILLION 1000000000

DiaConfiguration * config;

int _100MsIntervalsCount;
char devName[128];
char centralKey[12];

int _DebugKey = 0;
int _Balance = 0;
int GetKey(DiaGpio * _gpio) {
    int key =0;
#ifdef USE_GPIO
    key = DiaGpio_GetLastKey(_gpio);
#endif

#ifdef USE_KEYBOARD
    if(_DebugKey!=0) key = _DebugKey;
    _DebugKey = 0;
#endif
    //if(key == 7) key = 6;
    if (key) {
        printf("key %d reported\n", key);
    }
    return key;
}

DiaNetwork network("http://app.diae.ru:8001/v0/graphql");

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

int send_receipt(int postPosition, int isCard, int amount) {
    return network.ReceiptRequest(postPosition, isCard, amount);
}

int turn_program(void *object, int program) {
    #ifdef USE_GPIO
    DiaGpio * gpio = (DiaGpio *)object;
    // negative number will stop all
    if(program>=MAX_PROGRAMS_COUNT) {
        return 1;
    }
    gpio->CurrentProgram = program;
    #endif
    return 0;
}

void SaveIncome() {
    if (config && config->GetStorage()) {
        storage_interface_t * storage = config->GetStorage();
        storage->save(storage->object, "income", &(config->_Income), sizeof(income));
    }
}

int get_coins(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->CoinMoney + _Balance;

    manager->CoinMoney  = 0;
    _Balance = 0;
        
    if(curMoney>0) {
        printf("coin %d\n", curMoney);

        if(config) {
            config->_Income.totalIncomeCoins += curMoney;
            SaveIncome();
        }
    }
    return curMoney;
}

int get_banknotes(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->BanknoteMoney;
    if(curMoney>0) {
        printf("bank %d\n", curMoney);
        if(config) {
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
    if(curMoney>0) {
        printf("electron %d\n", curMoney);
        if(config) {
            config->_Income.totalIncomeElectron += curMoney;
            SaveIncome();
        }
        manager->ElectronMoney  = 0;
        _Balance = 0;
    }
    return curMoney;
}

int request_transaction(void *object, int money) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    if (money > 0) {
        DiaDeviceManager_PerformTransaction(manager, money);
        return 0;
    }
    return 1;
}

int get_transaction_status(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int status = DiaDeviceManager_GetTransactionStatus(manager);
    return status;
}

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

/*
inline int SendStatusRequest(const char * devName) {
    if (!config || !config->GetGpio()) {
        return 0;
    }

    _100MsIntervalsCount++;
    if(_100MsIntervalsCount<0) {
        printf("Memory corruption on _100MsIntervalsCount\n");
        _100MsIntervalsCount=0;
    }

    if(_100MsIntervalsCount>600) {
        _100MsIntervalsCount= 0;
        char timestamp_buf[32];
        timestamp_buf[0] = 0;
        sprintf(timestamp_buf,"%lu", (unsigned long)time(NULL));
        printf("sending status\n");

        network.create_money_report(0,devName,timestamp_buf, config->_Income.carsTotal,
        config->_Income.totalIncomeCoins, config->_Income.totalIncomeBanknotes,
        config->_Income.totalIncomeElectron, config->_Income.totalIncomeService );
        DiaGpio * gpio = config->GetGpio();
        if(config->GetStorage()){
            config->GetStorage()->save(config->GetStorage()->object, "relays", &(gpio->Stat), sizeof(gpio->Stat));
        }

        RelayStat_t RelayStats[MAX_RELAY_NUM];
        for(int i=0;i<MAX_RELAY_NUM;i++) {
            RelayStats[i].switched_count=gpio->Stat.relay_switch[i+1];
            RelayStats[i].total_time_on=gpio->Stat.relay_time[i+1]/1000;
        }
        network.create_relay_report(0,devName, timestamp_buf, RelayStats);
    }
    return 0;
}

int recover_money() {
    create_money_report_t* last_money_report=new create_money_report_t;
    int err = 0;
    err = network.get_last_money_report(last_money_report);
    if (err == 0) {
    fprintf(stderr,"id:%d transaction_id:%d StationPostID:%s TimeStamp:%s CarsTotal:%d CoinsTotal:%d BanknotesTotal:%d CashlessTotal:%d TestTotal:%d\n",last_money_report->id,
    last_money_report->transaction_id,last_money_report->station_post_id.c_str(),last_money_report->time_stamp.c_str(),last_money_report->cars_total,last_money_report->coins_total,
    last_money_report->banknotes_total,last_money_report->cashless_total,last_money_report->test_total);
    if ((config->_Income.totalIncomeCoins < last_money_report->coins_total) ||
    (config->_Income.totalIncomeBanknotes < last_money_report->banknotes_total) ||
    (config->_Income.totalIncomeElectron < last_money_report->cashless_total) ||
    (config->_Income.totalIncomeService < last_money_report->test_total) ||
    (config->_Income.carsTotal < last_money_report->cars_total)) {
        config->_Income.totalIncomeCoins = last_money_report->coins_total;
        config->_Income.totalIncomeBanknotes = last_money_report->banknotes_total;
        config->_Income.totalIncomeElectron = last_money_report->cashless_total;
        config->_Income.totalIncomeService = last_money_report->test_total;
        config->_Income.carsTotal = last_money_report->cars_total;
        SaveIncome();
    }
    } else {
        fprintf(stderr,"get last money report err:%d\n",err);
    }

    delete last_money_report;
    return err;
}

int recover_relay() {
    create_relay_report_t* last_relay_report=new create_relay_report_t;
    int err = 0;
    err = network.get_last_relay_report(last_relay_report);
    if (err == 0) {
        DiaGpio * gpio = config->GetGpio();
        fprintf(stderr,"id:%d transaction_id:%d StationPostID:%s TimeStamp:%s\n",last_relay_report->id,
        last_relay_report->transaction_id,last_relay_report->station_post_id.c_str(),last_relay_report->time_stamp.c_str());

        bool update = false;
        for(int i=0;i<MAX_RELAY_NUM;i++) {
            if ((gpio->Stat.relay_switch[i+1]<last_relay_report->RelayStats[i].switched_count) ||
                (gpio->Stat.relay_time[i+1]<last_relay_report->RelayStats[i].total_time_on*1000)) {
                    update = true;
            }
        }
        if (update) {
            fprintf(stderr,"update stat\n");
            for(int i=0;i<MAX_RELAY_NUM;i++) {
                gpio->Stat.relay_switch[i+1]=last_relay_report->RelayStats[i].switched_count;
                gpio->Stat.relay_time[i+1]=last_relay_report->RelayStats[i].total_time_on*1000;
            }
        }
    } else {
            fprintf(stderr,"get last relay report err:%d\n",err);
    }

    delete last_relay_report;
    return err;
}

int activate() {
    char buf_password[1024];
    char buf_username[1024];
    if (!file_exists(UNLOCK_KEY)) {
        printf("your firmware is NOT activated\n");
        printf("please type in your login:");
        scanf("%s", buf_username);
        printf("please type in your password:");
        scanf("%s", buf_password);
        int err = network.Login(buf_username, buf_password);
        if (err) {
            printf("Can't connect to the authority server, sorry. Please try again later (%d)\n", err );
            return 1;
        }

        std::string dev_password;
        std::string dev_key;
        err = network.RegisterPostWash(devName, dia_security_get_key(),
        &dev_password, &dev_key);
        if (err) {
            printf("Can't register your device (%d)\n", err);
            return 1;
        }
        dia_security_write_file(UNLOCK_KEY, dev_key.c_str());
        dia_security_write_file(DEVICE_PASS, dev_password.c_str());
    }
    return 0;
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

int RecoverRegistry() {
    printf("---START-----------------------------------------------------------------\n");
    Registries* MyRegistry= new Registries;
    int err = 0;
    err = network.MyRegistry(MyRegistry);
    
    DiaRuntimeRegistry* registry = &(config->GetRuntime()->Registry);
    
    if (err==0) { // Connection to CRM is OK
        fprintf(stderr, "Online registry: \n");
    
        for (auto it = MyRegistry->registries.begin(); it !=  MyRegistry->registries.end(); ++it) {
            registry->SetValue((*it).first.c_str(),(*it).second.c_str());
            fprintf(stderr, "%s:%s; \n",(*it).first.c_str(),(*it).second.c_str());
            
            std::string localData = GetLocalData((*it).first);
            if (localData != (*it).second) {
                SetLocalData((*it).first, (*it).second);
            }
        }
    } else { // Offline mode - need to check local data
        std::string default_price = "20";
        
        fprintf(stderr, "Local registry: \n");
        
        for (int i = 1; i < 7; i++) {
            std::string current_key = "price" + std::to_string(i);
            
            std::string value = GetLocalData(current_key);
            if (value == "0") {
                value = default_price;
            } 
            
            registry->SetValue(current_key.c_str(),value.c_str());
            fprintf(stderr, "%s:%s; \n",current_key.c_str(),value.c_str());
        } 
    }
    
    delete(MyRegistry);    
    printf("--------------------------------------------------------------------------\n");
    return err;
}
*/
int main(int argc, char ** argv) {
    config = 0;

    // Timer initialization
    struct timespec stored_time;
    clock_gettime(CLOCK_MONOTONIC_RAW, &stored_time);

    if (argc>2) {
        fprintf(stderr, "Too many parameters. Please leave just folder with the firmware, like [firmware.exe .] \n");
        return 1;
    }

    // Check public key on disk
    // If it doesn't exist - get it from MAC
    if (file_exists(CENTRALWASH_KEY)) {
        dia_security_read_file(CENTRALWASH_KEY, centralKey, sizeof(centralKey)+1);
        printf("Public key read from file: %s \n", centralKey);

    } else {
        network.GetMacAddress(centralKey);
        printf("MAC address: %s\n", centralKey);

        dia_security_write_file(CENTRALWASH_KEY, centralKey);
        printf("Public key wrote to file: %s \n", CENTRALWASH_KEY);
    }
    /*
    // Locate Central wash server 
    std::string serverIP = "localhost";
    int res = -1;

    while (res != 0) {
	    printf("Looking for central-wash service ...\n");
        res = network.PingServer(&serverIP);
        
        if (res == 0)
            printf("Server located on:\n%s\n", serverIP.c_str());
        else {
            printf("Failed... Next attempt soon...\n");
            sleep(60);
        }
    }
    network.OnlineCashRegister = serverIP;

    // Read ID from file
    strcpy(devName,"NO_ID");
    dia_security_read_file(ID_KEY, devName, sizeof(devName));
    for (unsigned int i=0;i<sizeof(devName);i++) {
        if (devName[i]=='\n' || devName[i]=='\r') {
            devName[i] = 0;
        }
    }
    printf("Device name: %s \n", devName);

    // Set working folder 
    std::string folder = "./firmware";
    if (argc == 2) {
        folder = argv[1];
    }
    printf("Looking for firmware in [%s]\n", folder.c_str());
    printf("Version: %s\n", DIA_VERSION);
    */
    /*
    int f = 1;
    int err = activate();

    if(err) {
        printf("activation error\n");
        return err;
    }

    if (file_exists(UNLOCK_KEY)) {
        char unlock_key[1024];
        dia_security_read_file(UNLOCK_KEY, unlock_key, sizeof(unlock_key));
        f = !dia_security_check_key(unlock_key);
    }
    char devPass[1024];
    printf("device name: [%s] \n", devName);
    strcpy(devPass, "NO_PASS");
    dia_security_read_file(DEVICE_PASS, devPass, sizeof(devPass));

    if (f) {
        printf(" DEMO MODE ...! \n");
    }

    err = network.Login(dia_security_get_key(), devPass);
    if (err) {
        printf("Can't login to diae server (%d)\n", err );
    }
    */
    /*
    // Runtime and firmware initialisation
    DiaDeviceManager manager;
    DiaDeviceManager_AddCardReader(&manager);

    SDL_Event event;

    DiaConfiguration configuration(folder);
    config = &configuration;
    err = configuration.Init();
    if (err!=0) {
        printf("Can't run due to the configuration error\n");
        return 1;
    }
    RecoverRegistry();
    recover_money();
    recover_relay();

    printf("Configuration is loaded...\n");

    // Screen load
    std::map<std::string, DiaScreenConfig *>::iterator it;
    for (it=configuration.ScreenConfigs.begin(); it!=configuration.ScreenConfigs.end() ;it++) {
        std::string currentID = it->second->id;
        DiaRuntimeScreen * screen = new DiaRuntimeScreen();
        screen->Name = currentID;
        screen->object = (void *)it->second;
        screen->set_value_function = dia_screen_config_set_value_function;
        screen->screen_object = configuration.GetScreen();
        screen->display_screen = dia_screen_display_screen;
        configuration.GetRuntime()->AddScreen(screen);
    }

    #ifdef USE_GPIO
    configuration.GetRuntime()->AddPrograms(&configuration.GetGpio()->_ProgramMapping);
    #else
    configuration.GetRuntime()->AddPrograms(0);
    #endif

    configuration.GetRuntime()->AddAnimations();

    ///// Runtime init
    DiaRuntimeHardware * hardware = new DiaRuntimeHardware();
    hardware->keys_object = configuration.GetGpio();
    hardware->get_keys_function = get_key;

    hardware->light_object = configuration.GetGpio();
    hardware->turn_light_function = turn_light;

    hardware->program_object = configuration.GetGpio();
    hardware->turn_program_function = turn_program;

    hardware->send_receipt_function = send_receipt;

    hardware->coin_object = &manager;
    hardware->get_coins_function = get_coins;

    hardware->banknote_object = &manager;
    hardware->get_banknotes_function = get_banknotes;

    hardware->electronical_object = &manager;
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
    configuration.GetRuntime()->Setup();
    while(!keypress)
    {
        configuration.GetRuntime()->Loop();
        SendStatusRequest(devName);
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    keypress = 1;
                    printf("quitting by sdl_quit\n");
                break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym)
                    {
                        case SDLK_UP:
                            _Balance+=10;
                            configuration._Income.totalIncomeService+=10;
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
                            printf("quitting by keypress...");
                            break;
                    }
                break;
            }
        }
    }
    */
    delay(2000);
    return 0;
}
