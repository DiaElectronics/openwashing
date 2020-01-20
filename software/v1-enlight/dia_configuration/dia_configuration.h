#ifndef DIA_CONFIGURATION_H
#define DIA_CONFIGURATION_H
#include <map>
#include <string>
#include <jansson.h>
#include "dia_screen_config.h"
#include "dia_screen.h"
#include "dia_runtime.h"
#include "dia_gpio.h"
#include "dia_program.h"

#define DIA_DEFAULT_FIRMWARE_FILENAME "main.json"

typedef struct income {
    double totalIncomeCoins;
    double totalIncomeBanknotes;
    double totalIncomeElectron;
    double totalIncomeService;
    double carsTotal;
} income_t;

class DiaConfiguration {
public:
    std::map<std::string, DiaScreenConfig *> ScreenConfigs;

    DiaConfiguration(std::string folder);
    ~DiaConfiguration();

    int Init();
    // how did they get here??? they must be runtime!
    // we just need to READ all configs here!
    int Setup();
    int Loop();
    int RunCommand(std::string command);
    // DELETE methods above
    DiaScreen * GetScreen() {
        assert(_Screen);
        return _Screen;
    }
    
    storage_interface_t *GetStorage() {
        assert(_Storage);
        return _Storage;
    }
    
    DiaRuntime *GetRuntime() {
        assert(_Runtime);
        return _Runtime;
    }
    
    DiaGpio *GetGpio() {
        #ifdef USE_GPIO
        assert(_Gpio);
        return _Gpio;
        #endif
        return 0;        
    }
    
    int GetResX() {
        assert(_ResX);
        return _ResX;
    }
    
    int GetResY() {
        assert(_ResY);
        return _ResY;
    }
    
    int GetButtonsNumber() {
        return _ButtonsNumber;
    }
    
    int GetRelaysNumber() {
        return _RelaysNumber;
    }
    
    std::string GetEndpoint() {
        return _Endpoint;
    }
    
    income_t _Income;
    
    private:
    std::string _Name;
    std::string _Folder;
    std::string _Endpoint;
    std::map<std::string, DiaProgram*> _Programs;
    
    DiaScreen * _Screen;
    DiaRuntime * _Runtime;
    DiaGpio * _Gpio;
    storage_interface_t * _Storage;
    
    
    int _ResX;
    int _ResY;
    int _ButtonsNumber;
    int _RelaysNumber;
    
    int InitFromFile();
    int InitFromString(const char * configuration_json);// never ever is going to be virtual
    int InitFromJson(json_t * configuration_json); //never ever virtual
};
#endif
