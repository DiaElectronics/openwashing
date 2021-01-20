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
    bool _isNegative;

public:
    DiaRuntimeSvcWeather(DiaNetwork * network) : _network{network} {
    }

    void SetCurrentTemperature() {
        printf("Requesting current temperature from the server...\n");
        assert(_network);
        std::string response = _network->GetRegistryValueByKey("curr_temp");
        auto decimal = response.find('.');
        auto degrees_value  = response.substr(0, decimal);
        auto fraction_value = response.substr(decimal+1, 1);

	    try {
            int degrees = std::stoi(degrees_value);
            if (degrees >= 0) {
                _isNegative = false;
                _temp_degrees = degrees;
            } else {
                _isNegative = true;
                _temp_degrees = 0 - degrees;
            }
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
        return _temp_degrees;
    }

    int GetTempFraction() {
        return _temp_fraction;
    }

    bool isNegative() {
        return _isNegative;
    }
};

#endif
