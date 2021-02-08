#ifndef dia_runtime_hardware_h
#define dia_runtime_hardware_h

#include "dia_functions.h"

#include <ctime>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaBridge.h"
#include <string>
#include <jansson.h>
#include <list>

using namespace luabridge;

class DiaRuntimeHardware {
public:
    std::string Name;

    void * light_object;
    int (*turn_light_function)(void * object, int pin, int animation_id);
    int TurnLight(int light_pin, int AnimationID) {
        //printf("light pin:[%d]->animation:[%d];\n", light_pin, AnimationID);
        if(light_object && turn_light_function) {
            turn_light_function(light_object, light_pin, AnimationID);
        } else {
            //printf("error: NIL object or function TurnLight\n");
        }
        return 0;
    }

    void * program_object;
    int (*turn_program_function)(void * object, int program);
    int TurnProgram(int ProgramNum) {
        if(program_object && turn_program_function) {
            turn_program_function(program_object, ProgramNum);
        } else {
            //printf("error: NIL object or function TurnActivator\n");
        }
        return 0;
    }

    int (*send_receipt_function)(int postPosition, int cash, int electronical);
    
    int SendReceipt(int postPosition, int cash, int electronical) {
        if(send_receipt_function) {
            send_receipt_function(postPosition, cash, electronical);
        } else {
            printf("error: NIL function SendReceipt\n");
        }
	    return 0;
    }

    int (*set_current_state_function)(int balance, int program);
    int SetCurrentState(int balance, int program) {
        if(set_current_state_function) {
            return set_current_state_function(balance, program);
        } else {
            printf("error: NIL object or function SetCurrentState\n");
        }
        return 0;
    }

    int (*increment_cars_function)();
    int IncrementCars() {
	if(increment_cars_function) {
	    increment_cars_function();
	} else {
	    printf("error: NIL function IncrementCars\n");
	}
	    return 0;
    }

    void * coin_object;
    int (*get_coins_function)(void * object);
    int GetCoins() {
        //printf("get coins;\n");
        if(coin_object && get_coins_function) {
            return get_coins_function(coin_object);
        } else {
            printf("error: NIL object or function GetCoins\n");
        }
        return 0;
    }

    void * banknote_object;
    int (*get_banknotes_function)(void * object);
    int GetBanknotes() {
        //printf("get banknotes;\n");
        if(banknote_object && get_banknotes_function) {
            return get_banknotes_function(banknote_object);
        } else {
            printf("error: NIL object or function GetBanknotes\n");
        }
        return 0;
    }

    int (*get_service_function)();
    int GetService() {
        if(get_service_function) {
            return get_service_function();
        } else {
            printf("error: NIL object or function GetService\n");
        }
        return 0;
    }

    int (*get_openlid_function)();
    int GetOpenLid() {
        if (get_openlid_function) {
            return get_openlid_function();
        } else {
            printf("error: NIL object or function GetOpenLid");
        }
        return 0;
    }

    void * electronical_object;
    int (*get_electronical_function)(void * object);
    int GetElectronical() {
        if(electronical_object && get_electronical_function) {
            return get_electronical_function(electronical_object);
        } else {
            printf("error: NIL object or function GetElectronical\n");
        }
        return 0;
    }

    int (*request_transaction_function)(void * object, int money);
    int RequestTransaction(int money) {
        if(electronical_object && request_transaction_function) {
            return request_transaction_function(electronical_object, money);
        } else {
            printf("error: NIL object or function RequestTransaction\n");
        }
        return 0;
    }

    int GetHours() {
        std::time_t t = std::time(0);   // get time now
        std::tm* now = std::localtime(&t);
        return now->tm_hour;
    }

    int GetMinutes() {
        std::time_t t = std::time(0);   // get time now
        std::tm* now = std::localtime(&t);
        return now->tm_min;
    }

    int (*get_transaction_status_function)(void * object);
    int GetTransactionStatus() {
        if(electronical_object && get_transaction_status_function) {
            return get_transaction_status_function(electronical_object);
        } else {
            printf("error: NIL object or function GetTransactionStatus\n");
        }
        return 0;
    }

    int (*abort_transaction_function)(void * object);
    int AbortTransaction() {
        if(electronical_object && abort_transaction_function) {
            return abort_transaction_function(electronical_object);
        } else {
            printf("error: NIL object or function AbortTransaction\n");
        }
        return 0;
    }

    void * keys_object;
    int (*get_keys_function)(void * object);
    
    int GetKey() {
        #ifndef USE_GPIO
        assert(get_keys_function);
        return get_keys_function(0);
        #endif
        if(keys_object && get_keys_function) {
            return get_keys_function(keys_object);
        }
        printf("error: NIL object or function GetKey\n");
        return 0;
    }

    void * delay_object;
    int (*smart_delay_function)(void * object, int milliseconds);
    int SmartDelay(int milliseconds) {
        if(delay_object && smart_delay_function) {
            return smart_delay_function(delay_object, milliseconds);
        } else {
            printf("error: NIL object or function SmartDelay\n");
        }
        return 0;
    }

    DiaRuntimeHardware() {
        light_object = 0;
        turn_light_function = 0;

        program_object = 0;
        turn_program_function = 0;

	    send_receipt_function = 0;

        coin_object = 0;
        get_coins_function = 0;

        banknote_object = 0;
        get_banknotes_function = 0;

        get_service_function = 0;
        get_openlid_function = 0;

        electronical_object = 0;
        get_electronical_function = 0;
        request_transaction_function = 0;
        get_transaction_status_function = 0;
        abort_transaction_function = 0;

        increment_cars_function = 0;

        keys_object = 0;
        get_keys_function = 0;

        delay_object = 0;
        smart_delay_function = 0;
        set_current_state_function = 0;
    }
};

#endif
