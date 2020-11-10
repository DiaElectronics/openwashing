#ifndef dia_runtime_registry_h
#define dia_runtime_registry_h

#include "dia_functions.h"
#include "dia_network.h"

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

// Main object for Client-Server communication.


class DiaRuntimeRegistry {
private:
    DiaNetwork * network;
public:
    int curPostID;
    DiaRuntimeRegistry(DiaNetwork * newNetwork) {
        curPostID = 0;
        network = newNetwork;
    }
    
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

    int SetPostID(int newPostID) {
        curPostID = newPostID;
        return 0;
    }

    int GetPostID() {
        return curPostID;
    }

    int SetValue(std::string key,std::string value) {
        values[key]= value;
        return 0;
    }

    std::string SetValueByKeyIfNotExists(std::string key, std::string value) {
        return network->SetRegistryValueByKeyIfNotExists(key, value);
    }

private:
    std::map<std::string, std::string> values;
};

#endif
