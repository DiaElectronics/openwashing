#ifndef _DIA_INT_PAIR_H_
#define _DIA_INT_PAIR_H_
#include <jansson.h>
#include <string>

class DiaIntPair {
public:
    int x;
    int y;
    int Init(json_t * int_pair_json);
    int Init(std::string newValue);
};
#endif