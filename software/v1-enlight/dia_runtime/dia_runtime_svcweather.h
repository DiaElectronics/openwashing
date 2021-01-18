#ifndef dia_runtime_svcweather_h
#define dia_runtime_svcweather_h

#include "dia_functions.h"

#include <ctime>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaBridge.h"
#include <string>
#include <list>

using namespace luabridge;

class DiaRuntimeSvcWeather {
private:
    DiaNetwork * _network;
    int _temp_degrees;
    int _temp_fraction;

public:
    DiaRuntimeSvcWeather(DiaNetwork * network) {
        _network = network;
    }

    void SetCurrentTemperature() {
        printf("Requesting current temperature from the server...\n");
        assert(_network);
        std::string response = _network->GetRegistryValueByKey("curr_temp");
        auto decimal = response.find('.');
        auto degrees_value  = response.substr(0, decimal);
        auto fraction_value = response.substr(decimal+1);

	    try {
            int degrees = std::stoi(degrees_value);
            _temp_degrees = degrees;
	    }
        catch (std::invalid_argument &err) {
            fprintf(stderr, "Value is not an int, returning ...\n");
            return;
        }

	    try {
            int fraction = std::stoi(fraction_value);
            _temp_fraction = fraction;
	    }
        catch (std::invalid_argument &err) {
            fprintf(stderr, "Value is not an int, returning ...\n");
            return;
        }
    }

    int GetTempDegrees() {
        // printf("Temp degrees: %d\n", _temp_degrees);
        return _temp_degrees;
    }

    int GetTempFraction() {
        // printf("Temp fraction: %d\n", _temp_fraction);
        return _temp_fraction;
    }
};

#endif
