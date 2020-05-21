#ifndef _DIA_STRING_H_
#define _DIA_STRING_H_

#include <jansson.h>
#include <string>

class DiaString {
public:
    std::string value;
    int Init(json_t * string_json);
    int Init(std::string newValue);
};

#endif