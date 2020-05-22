#include "dia_int_pair.h"
#include <string>

int DiaIntPair::Init(json_t * int_pair_json) {
    if (int_pair_json == 0) {
        printf("nil parameter passed to int pair init\n");
        return 1;
    }
    if (!json_is_string(int_pair_json)) {
        printf("int pair description is not a string");
    }
    std::string resPair = json_string_value(int_pair_json);
    return Init(resPair);
}

int DiaIntPair::Init(std::string newValue) {
    const char * cursor = newValue.c_str();
    int res[2];
    res[0] = 0;
    res[1] = 0;
    int N=12;
    int curCoordinate = 0;
    while (cursor[0]) {
        if (cursor[0]==';') {
            curCoordinate++;
            if(curCoordinate>1) {
                printf("too many ';' symbols in int pair description\n");
                return 1;
            }
        } else {
            res[curCoordinate]*=10;
            int add = cursor[0]-48;
            if (add<0 || add>9) {
                printf("wrong symbol '%X' in int pair description: '%s'\n",cursor[0], newValue.c_str());
                return 1;
            }
            res[curCoordinate]+=add;
        }
        cursor++;
        N--;
        if (N<0) {
            printf("error: too long pair description");
            return 1;
        }
    }
    x = res[0];
    y = res[1];

    return 0;
}
