#include "dia_boolean.h"
#include <string>
int DiaBoolean::Init(json_t * number_json) {
    if (number_json == 0) {
        printf("nil parameter passed to boolean init\n");
        return 1;
    }
    if (!json_is_string(number_json)) {
        printf("boolean description is not a string");
    }
    std::string resBoolean = json_string_value(number_json);

   return Init(resBoolean);
}

int DiaBoolean::Init(std::string newValue) {    

    value = 0;
    if (newValue.compare("false")==0) {
        value = 0;
    } else if (newValue.compare("true")==0) {
        value = 1;
    } else {
        printf("error: can't recognize true or false in '%s'\n", newValue.c_str());
        return 1;
    }
    
    return 0;
}