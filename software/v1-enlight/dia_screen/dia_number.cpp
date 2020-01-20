#include "dia_number.h"
#include <string>

int DiaNumber::Init(json_t * number_json) {
    if (number_json == 0) {
        printf("nil parameter passed to number init\n");
        return 1;
    }
    if (!json_is_string(number_json)) {
        printf("number description is not a string");
    }
    std::string resNumber = json_string_value(number_json);

    return Init(resNumber);
}

int DiaNumber::Init(std::string newValue) {

    const char * cursor = newValue.c_str();
    value = 0;
    int N=9;
    while (cursor[0]) {
        value*=10;
        int add = cursor[0]-48;
        if (add<0 || add>9) {
            printf("wrong symbol in int pair description\n");
        }
        value+=add;
        cursor++;
        N--;
        if (N<0) {
            printf("error: too long pair description");
            return 1;
        }
    }

    return 0;
}
