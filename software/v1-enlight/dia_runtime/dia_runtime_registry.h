#ifndef dia_runtime_registry_h
#define dia_runtime_registry_h

#include "dia_functions.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "LuaBridge.h"
#include <string>
#include <jansson.h>
#include <list>
#include <stdexcept>

using namespace luabridge;

class DiaRuntimeRegistry {
public:
    std::string Value(std::string key) {
        return values[key];
    }

    int ValueInt(std::string key) {
	int result = 0;

	try {
	    result = std::stoi(values[key]);
	}
	catch (std::invalid_argument &err) {
	    fprintf(stderr, "Key for registry is invalid, returning 0...\n");
	    return 0;
	}

	return result;
    }

    int SetValue(std::string key,std::string value) {
        values[key]= value;
        return 0;
    }

private:
    std::map<std::string, std::string> values;
};

#endif
