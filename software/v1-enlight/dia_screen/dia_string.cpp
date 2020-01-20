#include "dia_string.h"
#include <string>

int DiaString::Init(json_t * string_json) {
    if (string_json == 0) {
        printf("nil parameter passed to string init\n");
        return 1;
    }
    if (!json_is_string(string_json)) {
        printf("string description is not a string");
    }
    std::string resNumber = json_string_value(string_json);

    return Init(resNumber);
}

int DiaString::Init(std::string newValue) {
    value = newValue;
    return 0;
}
