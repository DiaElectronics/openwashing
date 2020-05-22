#ifndef _DIA_PROGRAM_H
#define _DIA_PROGRAM_H
#include <string>
#include <jansson.h>
#include "dia_relayconfig.h"
#include "dia_functions.h"

class DiaProgram {
    public:
    std::string ID;
    std::string Source;
    int _InitializedOk;
    
    DiaRelayConfig Config;
    
    DiaProgram(json_t * program_node, std::string currentFolder) {
        _InitializedOk = 0;
        // Let's unpack buttons #
        json_t *id_json = json_object_get(program_node, "id");
        if(!json_is_string(id_json)) {
            fprintf(stderr, "error: id is not string for program \n");
            return;
        }
        ID = json_string_value(id_json);
        
        json_t *src_json = json_object_get(program_node, "src");
        if(!json_is_string(src_json)) {
            fprintf(stderr, "error: src is not string for program \n");
            return;
        }
        Source = json_string_value(src_json);
        int err = _LoadProgram(currentFolder, Source.c_str());
        if (err ==0 ) {
            _InitializedOk = 1;
        }
    }
    
    int _LoadProgram(std::string folder, const char * source) {
        json_t * program_src = dia_get_resource_json(folder.c_str(), source);
        if (program_src==0) {
            printf("program file [%s] is not found in folder [%s]\n",source, folder.c_str());
            return 1;
        }
        json_t *relays_src = json_object_get(program_src, "relays");
        if (program_src==0) {
            printf("relays element is not found in file [%s], folder [%s]\n",source, folder.c_str());
            return 1;
        }
        if(!json_is_array(relays_src)) {
            printf("relays element must be an array in file [%s], folder [%s]\n",source, folder.c_str());
            return 1;
        }
        for (unsigned int i=0;i<json_array_size(relays_src); i++) {
            //printf("relays reading loop\n");
            json_t * relay_json = json_array_get(relays_src, i);
            if(!json_is_object(relay_json)) {
                printf("relays element %d is not an object in file [%s], folder [%s]\n", i, source, folder.c_str());
                return 1;
            }
            int err = _ReadRelay(relay_json);
            if (err) {
                return 1;
            }
        }
        return 0;
    }
    int _ReadRelay(json_t *relay_json) {
        assert(relay_json);
        json_t * id_json =json_object_get(relay_json, "id");
        if (!json_is_integer(id_json)) {
            printf("relay id must be integer\n");
            return 1;
        }
        int id = json_integer_value(id_json);
        int ontime = 1000;
        int offtime = 0;
        
        json_t * ontime_json = json_object_get(relay_json, "ontime");
        json_t * offtime_json = json_object_get(relay_json, "offtime");
        if (json_is_integer(ontime_json) && json_is_integer(offtime_json)) {
            ontime = json_integer_value(ontime_json);
            offtime = json_integer_value(offtime_json);
        }
        int err = Config.InitRelay(id, ontime, offtime);
        
        return err;
    }
};

#endif
