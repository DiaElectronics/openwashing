#ifndef _DIA_PROGRAM_H
#define _DIA_PROGRAM_H
#include <string>
#include <jansson.h>
#include "dia_relayconfig.h"
#include "dia_functions.h"

class DiaProgram {
    public:
    int _InitializedOk;

    int ButtonID;
    int ProgramID;
    std::string Name;
    int Price;
    int MotorSpeedPercent;
    DiaRelayConfig Relays;
    bool PreflightEnabled;
    int PreflightMotorSpeedPercent;
    DiaRelayConfig PreflightRelays;
    
    DiaProgram(json_t * program_node) {
        _InitializedOk = 0;
        // Let's unpack buttons #
        json_t *id_json = json_object_get(program_node, "buttonID");
        if(!json_is_integer(id_json)) {
            fprintf(stderr, "error: buttonID is not int for program \n");
            return;
        }
        ButtonID = json_integer_value(id_json);

        program_node = json_object_get(program_node, "program");
        if(!json_is_object(program_node)) {
            printf("program is not an object\n");
            return;
        }

        json_t *program_id_json = json_object_get(program_node, "id");
        if(!json_is_integer(program_id_json)) {
            fprintf(stderr, "error: programID is not int for program \n");
            return;
        }
        ProgramID = json_integer_value(program_id_json);

        json_t *price_json = json_object_get(program_node, "price");
        if(!json_is_integer(price_json)) {
            fprintf(stderr, "error: price is not int for program \n");
            return;
        }
        Price = json_integer_value(price_json);

        json_t *motor_speed_percent_json = json_object_get(program_node, "motorSpeedPercent");
        if(json_is_integer(motor_speed_percent_json)) {
            MotorSpeedPercent = json_integer_value(motor_speed_percent_json);
        }

        json_t *preflight_motor_speed_percent_json = json_object_get(program_node, "preflightMotorSpeedPercent");
        if(json_is_integer(preflight_motor_speed_percent_json)) {
            PreflightMotorSpeedPercent = json_integer_value(preflight_motor_speed_percent_json);
        }

        json_t *preflight_enabled_json = json_object_get(program_node, "preflightEnabled");
        if(json_is_boolean(preflight_enabled_json)) {
            PreflightEnabled = json_boolean_value(preflight_enabled_json);
        }

        json_t *name_json = json_object_get(program_node, "name");
        if(json_is_string(name_json)) {
            Name = json_string_value(name_json);
        }
        printf("program %s, buttonID %d, programID %d, motorSpeed %d, PreflightEnabled %d, PreflightMotorSpeed %d\n", Name.c_str(),ButtonID,ProgramID,MotorSpeedPercent,PreflightEnabled,PreflightMotorSpeedPercent);
        int err = _LoadRelays(json_object_get(program_node, "relays"), false);
        if (err !=0 ) {
            return;
        }
        if (PreflightEnabled) {
            err = _LoadRelays(json_object_get(program_node, "preflightRelays"), true);
        }
        if (err ==0 ) {
            _InitializedOk = 1;
        }
    }
    
    int _LoadRelays(json_t * relays_src, bool preflight) {
        if(!json_is_array(relays_src)) {
            printf("relays element must be an array\n");
            return 1;
        }
        for (unsigned int i=0;i<json_array_size(relays_src); i++) {
            //printf("relays reading loop\n");
            json_t * relay_json = json_array_get(relays_src, i);
            if(!json_is_object(relay_json)) {
                printf("relays element %d is not an object\n", i);
                return 1;
            }
            int err = _ReadRelay(relay_json, preflight);
            if (err) {
                return 1;
            }
        }
        return 0;
    }
    int _ReadRelay(json_t *relay_json, bool preflight) {
        assert(relay_json);
        json_t * id_json =json_object_get(relay_json, "id");
        if (!json_is_integer(id_json)) {
            printf("relay id must be integer\n");
            return 1;
        }
        int id = json_integer_value(id_json);
        int ontime = 1000;
        int offtime = 0;
        
        json_t * ontime_json = json_object_get(relay_json, "timeon");
        json_t * offtime_json = json_object_get(relay_json, "timeoff");
        if (json_is_integer(ontime_json) && json_is_integer(offtime_json)) {
            ontime = json_integer_value(ontime_json);
            offtime = json_integer_value(offtime_json);
        }
        printf("relay id %d, preflight %d, on %d, off %d\n",id, preflight, ontime, offtime);
        int err = 0;
        if (preflight) {
            err = PreflightRelays.InitRelay(id, ontime, offtime);
        } else {
            err = Relays.InitRelay(id, ontime, offtime);
        }
        return err;
    }
};

#endif
