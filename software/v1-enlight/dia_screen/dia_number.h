#ifndef _DIA_NUMBER_H_
#define _DIA_NUMBER_H_

#include <jansson.h>
#include <string>

class DiaNumber {
public:
    int value;
    int Init(json_t * number_json);
    int Init(std::string newValue);
};

#endif