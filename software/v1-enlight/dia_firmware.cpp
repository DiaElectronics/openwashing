#include <stdio.h>
#include <wiringPi.h>

#include "dia_gpio.h"
#include "dia_screen.h"
#include "dia_devicemanager.h"
#include "dia_network.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <sys/file.h>
#include "dia_security.h"
#include "dia_functions.h"
#include "dia_configuration.h"
#include "dia_runtime.h"
#include "dia_runtime_registry.h"
#include <string>
#include <map>
#include <time.h>
#include <unistd.h>
#include "dia_screen_item.h"
#include "dia_screen_item_image.h"


#define DIA_VERSION "v1.8-enlight"


//#define USE_GPIO
#define USE_KEYBOARD


// TODO: must be set via API
#define COIN_MULTIPLICATOR 10
#define BANKNOTE_MULTIPLICATOR 100
#define ALLOW_PULSE 1
// END must be set via API

#define BILLION 1000000000

DiaConfiguration * config;

int _IntervalsCount;
int _IntervalsCountRelay;

// Public key for signing every request to Central Server.
const int centralKeySize = 6;
std::string centralKey;

int _DebugKey = 0;

// Variable for storing an additional money.
// For instance, service money from Central Server can be transfered inside.
int _Balance = 0;
int _OpenLid = 0;

int _to_be_destroyed = 0;

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
DiaNetwork * network = new DiaNetwork();

// Saves new income money and creates money report to Central Server.
void SaveIncome(int cars_total, int coins_total, int banknotes_total, int cashless_total, int service_total) {
        network->SendMoneyReport(cars_total,
        coins_total,
        banknotes_total,
        cashless_total,
        service_total);
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

pthread_t pinging_thread;

// Creates receipt request to Online Cash Register.
int send_receipt(int postPosition, int isCard, int amount) {
    return network->ReceiptRequest(postPosition, isCard, amount);
}

// Increases car counter in config
int increment_cars() {
    printf("Cars incremented\n");
    SaveIncome(1,0,0,0,0);
    return 0;
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

int get_service() {
    int curMoney = _Balance;
    _Balance = 0;
        
    if (curMoney > 0) {
        printf("service %d\n", curMoney);
        SaveIncome(0,0,0,0,curMoney);
    }
    return curMoney;
}

int get_openlid() {
    int curOpenLid = _OpenLid;
    _OpenLid = 0;
   
    return curOpenLid;
}

int get_coins(void *object) {
  DiaDeviceManager * manager = (DiaDeviceManager *)object;
  int curMoney = manager->CoinMoney;
  manager->CoinMoney  = 0;  
  int gpioCoin = 0;

  if (ALLOW_PULSE && config) {
    DiaGpio *g = config->GetGpio();
    if (g) {
      gpioCoin = COIN_MULTIPLICATOR * g->CoinsHandler->Money;
      g->CoinsHandler->Money = 0;
    }
  }

  int gpioCoinAdditional = 0;

  if (ALLOW_PULSE && config) {
    DiaGpio *g = config->GetGpio();
    if (g && g->AdditionalHandler) {
      gpioCoinAdditional = COIN_MULTIPLICATOR * g->AdditionalHandler->Money;
      g->AdditionalHandler->Money = 0;
    }
  }

  if (curMoney > 0) printf("coins from manager %d\n", curMoney);
  if (gpioCoin > 0) printf("coins from gpio %d\n", gpioCoin);
  if (gpioCoinAdditional > 0) printf("coins from additional gpio %d\n", gpioCoinAdditional);

  int totalMoney = curMoney + gpioCoin + gpioCoinAdditional;
  if (curMoney>0) {
      SaveIncome(0,curMoney,0,0,0);
  }

  return totalMoney;
}

int get_banknotes(void *object) {
  DiaDeviceManager * manager = (DiaDeviceManager *)object;
  int curMoney = manager->BanknoteMoney;
  manager->BanknoteMoney = 0;

  int gpioBanknote = 0;
  if (ALLOW_PULSE && config) {
    DiaGpio *g = config->GetGpio();
    if (g) {
      gpioBanknote = BANKNOTE_MULTIPLICATOR * g->BanknotesHandler->Money;
      g->BanknotesHandler->Money = 0;
    }
  }

  if (curMoney > 0) printf("banknotes from manager %d\n", curMoney);
  if (gpioBanknote > 0) printf("banknotes from GPIO %d\n", gpioBanknote);
  int totalMoney = curMoney + gpioBanknote;
  if (curMoney > 0) {
    SaveIncome(0,0,curMoney,0,0);
  }
  return totalMoney;
}

int get_electronical(void *object) {
    DiaDeviceManager * manager = (DiaDeviceManager *)object;
    int curMoney = manager->ElectronMoney;
    if (curMoney>0) {
        printf("electron %d\n", curMoney);
        SaveIncome(0,0,0,curMoney,0);
        manager->ElectronMoney  = 0;
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
    printf("PING CENTRAL SERVER\n");
    
    _IntervalsCount++;
    if(_IntervalsCount < 0) {
        printf("Memory corruption on _IntervalsCount\n");
        _IntervalsCount = 0;
    }

    _IntervalsCountRelay++;
    if(_IntervalsCountRelay < 0) {
        printf("Memory corruption on _IntervalsCountRelay\n");
        _IntervalsCountRelay = 0;
    }

    printf("Sending another PING request to server...\n");

    int serviceMoney = 0;
    bool openStation = false;
    network->SendPingRequest(network->GetHostName(), serviceMoney, openStation);
    
    if (serviceMoney > 0) {
        // TODO protect with mutex
        _Balance += serviceMoney;
    }
    if (openStation) {
        _OpenLid = _OpenLid + 1;
        printf("Door is going to be opened... \n");
        // TODO: add the function of turning on the relay, which will open the lock.
    }
    

    // Every 5 min (300 sec) we go inside this
    if (_IntervalsCountRelay > 300) {
        _IntervalsCountRelay = 0;

        if (config->GetGpio()) {
            printf("Sending relay report to server...\n");
        
            RelayStat *relays = new RelayStat[MAX_RELAY_NUM];
            DiaGpio * gpio = config->GetGpio();
            for (int i = 0; i < MAX_RELAY_NUM; i++) {
                relays[i].switched_count = gpio->Stat.relay_switch[i+1];
                relays[i].total_time_on = gpio->Stat.relay_time[i+1];
            }
            network->SendRelayReport(relays);
            delete[] relays;
        }
    }

    {   // Every 30 min (1800 sec) we go inside this
        static const int maxIntervalsCountWeather = 1800;
        static int intervalsCountWeather = maxIntervalsCountWeather;
        if (intervalsCountWeather >= maxIntervalsCountWeather) {
            intervalsCountWeather = 0;
            config->GetSvcWeather()->SetCurrentTemperature();
        }
        ++intervalsCountWeather;
    }
    return 0;
}

void * pinging_func(void * ptr) {
    while(!_to_be_destroyed) {
        CentralServerDialog();
        sleep(1);
    }
    pthread_exit(0);
    return 0;
}

int RecoverRelay() {
    relay_report_t* last_relay_report = new relay_report_t;

    int err = network->GetLastRelayReport(last_relay_report);

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
        char value[6];
        dia_security_read_file(filename.c_str(), value, 5);

        return std::string(value, 6);
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

    DiaRuntimeRegistry* registry = config->GetRuntime()->Registry;

    std::string value = "";

    int tmp = 0;
    bool openStation = false;
    
    std::string default_price = "15";
    int err = 1;
    while (err) {
        err = network->SendPingRequest(network->GetHostName(), tmp, openStation);
        if (err) {
            printf("waiting for server proper answer \n");
            sleep(5);
            continue;
        }
        // Load all prices in online mode
        fprintf(stderr, "Online mode, registry got from Central Server...\n");

        for (int i = 1; i < 7; i++) {
            std::string key = "price" + std::to_string(i);
            std::string value = network->GetRegistryValueByKey(key);

            if (value != "") {
                fprintf(stderr, "Key-value read online => %s:%s; \n", key.c_str(), value.c_str());
            } else {
		        fprintf(stderr, "Server returned empty value, setting default...\n");
		        value = default_price;
		        network->SetRegistryValueByKeyIfNotExists(key, value);
	        }
	        registry->SetValue(key.c_str(), value.c_str());
        }
    }
    return err;
}
//////////////////////////////////////////////

// Just compilation of recovers.
void RecoverData() {
    RecoverRegistry();
    // RecoverMoney();
    RecoverRelay();
}

int onlyOneInstanceCheck() {
  int socket_desc;
  socket_desc=socket(AF_INET,SOCK_STREAM,0);
  if (socket_desc==-1) {
    perror("Create socket");
  }
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  //Port defined Here:
  address.sin_port=htons(2223);
  //Bind
  int res = bind(socket_desc,(struct sockaddr *)&address,sizeof(address));
  if (res < 0) {
      printf("bind failed :(\n");
      return 0;
  }
  listen(socket_desc,32);
//Do other stuff (includes accepting connections)

  return 1;
}

int main(int argc, char ** argv) {
    config = 0;
    if (!onlyOneInstanceCheck()) {
        printf("sorry, just one instance of the application allowed\n");
        return 0;
    }

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
 
    centralKey = network->GetMacAddress(centralKeySize);
    network->SetPublicKey(std::string(centralKey));

    printf("MAC address or KEY: %s\n", centralKey.c_str());
    
    int need_to_find = 1; 
    std::string serverIP = "";

    while (need_to_find) {
    	serverIP = network->GetCentralServerAddress();
    	if (serverIP == "") {
        	printf("Error: Center Server is unavailable. Next try...\n");
    	} else
	    need_to_find = 0;
    }
    network->SetHostAddress(serverIP);
    std::string stationIDasString = network->GetStationID();
    int stationID = 0;
    try { 
        stationID = std::stoi(stationIDasString);
    }
    catch(...) {
        printf("Wrong post number received [%s]\n", stationIDasString.c_str() );
    }

    // Runtime and firmware initialization
    DiaDeviceManager *manager = new DiaDeviceManager;
    DiaDeviceManager_AddCardReader(manager);

    SDL_Event event;

    config = new DiaConfiguration(folder, network);
    int err = config->Init();
    if (err != 0) {
        printf("Can't run due to the configuration error\n");
        return 1;
    }
    
    // Get working data from server: money, relays, prices
    RecoverData();
 
    printf("Configuration is loaded...\n");

    // Screen load
    
    std::map<std::string, DiaScreenConfig *>::iterator it;
    for (it = config->ScreenConfigs.begin(); it != config->ScreenConfigs.end(); it++) {
        std::string currentID = it->second->id;
        DiaRuntimeScreen * screen = new DiaRuntimeScreen();
        screen->Name = currentID;
        screen->object = (void *)it->second;
        screen->set_value_function = dia_screen_config_set_value_function;
        screen->screen_object = config->GetScreen();
        screen->display_screen = dia_screen_display_screen;
        config->GetRuntime()->AddScreen(screen);
    }
    
    // Program load
    
    #ifdef USE_GPIO
    config->GetRuntime()->AddPrograms(&config->GetGpio()->_ProgramMapping);
    #else
    printf("NOT USING GPIO - adding FAKE programs...\n");
    std::map<std::string, int> *fake_programs = new std::map<std::string, int>();
    fake_programs->insert( std::pair<std::string, int>("p1relay", 1) ); 
    fake_programs->insert( std::pair<std::string, int>("p2relay", 2) );
    fake_programs->insert( std::pair<std::string, int>("p3relay", 3) );
    fake_programs->insert( std::pair<std::string, int>("p4relay", 4) );
    fake_programs->insert( std::pair<std::string, int>("p5relay", 5) );
    fake_programs->insert( std::pair<std::string, int>("p6relay", 6) );
    fake_programs->insert( std::pair<std::string, int>("openlid", 7) );
    fake_programs->insert( std::pair<std::string, int>("p7relay", 7) );
    fake_programs->insert( std::pair<std::string, int>("p8relay", 8) );
    config->GetRuntime()->AddPrograms(fake_programs);
    #endif

    config->GetRuntime()->AddAnimations();
  
    DiaRuntimeHardware * hardware = new DiaRuntimeHardware();
    hardware->keys_object = config->GetGpio();
    hardware->get_keys_function = get_key;

    hardware->light_object = config->GetGpio();
    hardware->turn_light_function = turn_light;

    hardware->program_object = config->GetGpio();
    hardware->turn_program_function = turn_program;

    hardware->send_receipt_function = send_receipt;
    hardware->increment_cars_function = increment_cars;

    hardware->coin_object = manager;
    hardware->get_coins_function = get_coins;

    hardware->banknote_object = manager;
    hardware->get_banknotes_function = get_banknotes;

    hardware->electronical_object = manager;
    hardware->get_service_function = get_service;
    hardware->get_openlid_function = get_openlid;
    hardware->get_electronical_function = get_electronical;    
    hardware->request_transaction_function = request_transaction;  
    hardware->get_transaction_status_function = get_transaction_status;
    hardware->abort_transaction_function = abort_transaction;

    hardware->delay_object = &stored_time;
    hardware->smart_delay_function = smart_delay_function;

    config->GetRuntime()->AddHardware(hardware);
    config->GetRuntime()->AddRegistry(config->GetRuntime()->Registry);
    config->GetRuntime()->AddSvcWeather(config->GetSvcWeather());
    config->GetRuntime()->Registry->SetPostID(stationID);
    
    //InitSensorButtons();

    // Runtime start
    int keypress = 0;
    int mousepress = 0;

    // Call Lua setup function
    config->GetRuntime()->Setup();

    // using button as pulse is a crap obviously
    if (config->UseLastButtonAsPulse() && config->GetGpio()) {
        printf("enabling additional coin handler\n");
        DiaGpio_StartAdditionalHandler(config->GetGpio());
    } else {
        printf("no additional coin handler\n");
    }

    pthread_create(&pinging_thread, NULL, pinging_func, NULL);
    while(!keypress) {
        // Call Lua loop function
        config->GetRuntime()->Loop();

        int x = 0;
        int y = 0;
        SDL_GetMouseState(&x, &y);
        if (config->NeedRotateTouch()) {
            x = config->GetResX() - x;
            y = config->GetResY() - y;
        }

        
        // Process pressed button
        DiaScreen* screen = config->GetScreen();
        std::string last = screen->LastDisplayed;

        for (auto it = config->ScreenConfigs[last]->clickAreas.begin(); it != config->ScreenConfigs[last]->clickAreas.end(); ++it) {
            if (x >= (*it).X && x <= (*it).X + (*it).Width && y >= (*it).Y && y <= (*it).Y + (*it).Height && mousepress == 1) {
                printf("CLICK!!!\n");
                mousepress = 0;
                _DebugKey = std::stoi((*it).ID);
                printf("DEBUG KEY = %d\n", _DebugKey);
            }
        }
        
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    keypress = 1;
                    printf("Quitting by sdl_quit\n");
                break;
                case SDL_MOUSEBUTTONDOWN:
                    mousepress = 1;
                break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_UP:
                            // Debug service money addition
                            _Balance += 10;

                            printf("UP\n"); fflush(stdout);
                            break;
                        case SDLK_DOWN:
                            // Debug service money addition
                            _Balance += 1;

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
    _to_be_destroyed = 1;

    delay(2000);
    return 0;
}

std::string SetRegistryValueByKeyIfNotExists(std::string key, std::string value) {
    return network->SetRegistryValueByKeyIfNotExists(key, value);
    }
