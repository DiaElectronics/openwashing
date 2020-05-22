#ifndef _DIA_BOOLEAN_H_
#define _DIA_BOOLEAN_H_

#include <jansson.h>
#include <string>

class DiaBoolean {
public:
    int value;
    int Init(json_t * boolean_json);
    int Init(std::string newValue);
    DiaBoolean() {
        value = 0;
    }
};

#endif
