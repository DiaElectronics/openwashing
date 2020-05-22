#ifndef dia_runtime_h
#define dia_runtime_h

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

#include "dia_runtime_screen.h"
#include "dia_runtime_hardware.h"
#include "dia_runtime_registry.h"

using namespace luabridge;


class DiaRuntime {
public:
    std::string src;
    lua_State* Lua;
    int Setup();
    int Loop();
    DiaRuntime();
    ~DiaRuntime();
    LuaRef * SetupFunction;
    LuaRef * LoopFunction;
    std::list<DiaRuntimeScreen *> all_screens;
    DiaRuntimeHardware * hardware;
    DiaRuntimeRegistry Registry;
    int Init(std::string folder, json_t * src_json);
    int Init(std::string folder, std::string src_str);
    int AddScreen(DiaRuntimeScreen * screen);
    int AddHardware(DiaRuntimeHardware * hw);
    int AddRegistry(DiaRuntimeRegistry * hw);
    int AddAnimations();
    int AddPrograms(std::map<std::string, int> *programs);
};

void printMessage(const std::string& s);

#endif
