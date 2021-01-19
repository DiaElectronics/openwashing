#include "dia_runtime.h"
#include <string>

int DiaRuntime::Init(std::string folder, json_t * src_json, json_t * include_json) {
    hardware = 0;
    if(src_json == 0) {
        printf("error: script src is null\n");
        return 1;
    }

    if(!json_is_string(src_json)) {
        printf("error: script src is not string\n");
        return 1;
    }

    std::string srcNew = json_string_value(src_json);
    std::string includeNew = "";

    // Let's do the same with include file
    if(include_json != 0) {
        printf("include file is in use\n");
        if(!json_is_string(include_json)) {
            printf("error: include json is not string\n");
            return 1;
        }
        includeNew = json_string_value(include_json);
    }

    

    return InitStr(folder, srcNew, includeNew);
}

void printMessage(const std::string& s) {
    fprintf(stderr, "%s\n", s.c_str());
}

int DiaRuntime::InitStr(std::string folder, std::string src_str, std::string incl_str) {
    src = src_str;
    incl = incl_str;
    std::string script_body = dia_get_resource(folder.c_str(), src.c_str());
    std::string include_body = "";

    if (!incl_str.empty()) {
        include_body = dia_get_resource(folder.c_str(), incl.c_str());
    }

    script_body += "\n\n";
    script_body += include_body;
    Lua = luaL_newstate();
    luaL_openlibs(Lua);

    getGlobalNamespace(Lua).addFunction("printMessage", printMessage);

    luaL_dostring(Lua, script_body.c_str());

    LuaRef setupFunction = getGlobal(Lua, "setup");
    if (setupFunction.isNil()) {
        printf("\n\n\nERROR: Setup function is not defined\n\n\n");
        return 1;
    } else {
        // printf("log: setup function is found\n");
    }
    SetupFunction = new LuaRef(Lua);
    *SetupFunction = setupFunction;

    LuaRef loopFunction = getGlobal(Lua, "loop");
    if (loopFunction.isNil()) {
        printf("\n\n\nERROR: Loop function is not defined\n\n\n");
        return 1;
    } else {
        //printf("log: loop function is found\n");
    }

    LoopFunction = new LuaRef(Lua);
    *LoopFunction = loopFunction;

    printf("RUNTIME'S PROPERLY INITIALIZED..\n");
    getGlobalNamespace(Lua)
    .beginClass<DiaRuntimeScreen>("DiaRuntimeScreen")
    .addConstructor<void(*)()>()
    .addFunction("Display", &DiaRuntimeScreen::Display)
    .addFunction("Set", &DiaRuntimeScreen::SetValue)
    .endClass();

    getGlobalNamespace(Lua)
    .beginClass<DiaRuntimeHardware>("DiaRuntimeHardware")
    .addConstructor<void(*)()>()
    .addFunction("TurnLight", &DiaRuntimeHardware::TurnLight)
    .addFunction("TurnProgram", &DiaRuntimeHardware::TurnProgram)
    .addFunction("GetCoins", &DiaRuntimeHardware::GetCoins)
    .addFunction("GetBanknotes", &DiaRuntimeHardware::GetBanknotes)
    .addFunction("GetService", &DiaRuntimeHardware::GetService)
    .addFunction("GetOpenLid", &DiaRuntimeHardware::GetOpenLid)
    .addFunction("SmartDelay", &DiaRuntimeHardware::SmartDelay)
    .addFunction("GetKey", &DiaRuntimeHardware::GetKey)
    .addFunction("SendReceipt", &DiaRuntimeHardware::SendReceipt)
    .addFunction("IncrementCars", &DiaRuntimeHardware::IncrementCars)
    .addFunction("GetElectronical", &DiaRuntimeHardware::GetElectronical)
    .addFunction("RequestTransaction", &DiaRuntimeHardware::RequestTransaction)
    .addFunction("GetHours", &DiaRuntimeHardware::GetHours)
    .addFunction("GetMinutes", &DiaRuntimeHardware::GetMinutes)
    .addFunction("GetTransactionStatus", &DiaRuntimeHardware::GetTransactionStatus)
    .addFunction("AbortTransaction", &DiaRuntimeHardware::AbortTransaction)
    .endClass();

    getGlobalNamespace(Lua)
    .beginClass<DiaRuntimeRegistry>("DiaRuntimeRegistry")
    .addFunction("Value", &DiaRuntimeRegistry::Value)
    .addFunction("ValueInt", &DiaRuntimeRegistry::ValueInt)
    .addFunction("SetValueByKeyIfNotExists", &DiaRuntimeRegistry::SetValueByKeyIfNotExists)
    .addFunction("GetPostID", &DiaRuntimeRegistry::GetPostID)
    .endClass();

    getGlobalNamespace(Lua)
    .beginClass<DiaRuntimeSvcWeather>("DiaRuntimeSvcWeather")
    .addFunction("GetTempDegrees", &DiaRuntimeSvcWeather::GetTempDegrees)
    .addFunction("GetTempFraction", &DiaRuntimeSvcWeather::GetTempFraction)
    .addFunction("IsNegative", &DiaRuntimeSvcWeather::isNegative)
    .addFunction("HasTwoDigits", &DiaRuntimeSvcWeather::hasTwoDigits)
    .endClass();

    return 0;
}

int DiaRuntime::AddScreen(DiaRuntimeScreen * screen) {
    assert(screen);
    all_screens.push_back(screen);
    luabridge::push(Lua, screen);
    lua_setglobal(Lua, screen->Name.c_str());
    //printf("added screen [%s]\n", screen->Name.c_str());
    return 0;
}

int DiaRuntime::AddPrograms(std::map<std::string, int> *programs) {
    lua_newtable(Lua);

    lua_pushliteral(Lua, "stop");
    lua_pushinteger(Lua, -1);
    lua_settable(Lua, -3);

    if (programs) {
        for(auto it_p = programs->begin(); it_p != programs->end(); it_p++) {
            lua_pushstring(Lua, it_p->first.c_str());
            lua_pushinteger(Lua, it_p->second);
            lua_settable(Lua, -3);
            printf("added program [%s] as [%d] \n", it_p->first.c_str(), it_p->second);
        }
    } 

    lua_setglobal(Lua, "program");

    return 0;
}

int DiaRuntime::AddAnimations() {
    lua_newtable(Lua);

    lua_pushliteral(Lua, "stop");
    lua_pushinteger(Lua, 0);
    lua_settable(Lua, -3);

    lua_pushliteral(Lua, "one_button");
    lua_pushinteger(Lua, 1);
    lua_settable(Lua, -3);

    lua_pushliteral(Lua, "idle");
    lua_pushinteger(Lua, 2);
    lua_settable(Lua, -3);

    lua_pushliteral(Lua, "intense");
    lua_pushinteger(Lua, 3);
    lua_settable(Lua, -3);

    lua_pushliteral(Lua, "freeze");
    lua_pushinteger(Lua, 4);
    lua_settable(Lua, -3);

    lua_setglobal(Lua, "animation");
    return 0;
}

int DiaRuntime::AddHardware(DiaRuntimeHardware * hw) {
    luabridge::push(Lua, hw);
    lua_setglobal(Lua, "hardware");
    printf("added hardware\n");
    return 0;
}

int DiaRuntime::AddRegistry(DiaRuntimeRegistry * reg) {
    luabridge::push(Lua, reg);
    lua_setglobal(Lua, "registry");
    printf("added registry\n");
    return 0;
}

int DiaRuntime::AddSvcWeather(DiaRuntimeSvcWeather * svc) {
    luabridge::push(Lua, svc);
    lua_setglobal(Lua, "weather");
    printf("added weather\n");
    return 0;
}

int DiaRuntime::Setup() {
    int result = (*SetupFunction)();
    return result;
}

int DiaRuntime::Loop() {
    int result = (*LoopFunction)();
    return result;
}

DiaRuntime::DiaRuntime(DiaRuntimeRegistry *newDiaRuntimeRegistry) {
    Lua = 0;
    SetupFunction = 0;
    LoopFunction = 0;
    Registry = newDiaRuntimeRegistry;
}

DiaRuntime::~DiaRuntime() {
    if (SetupFunction) {
        delete SetupFunction;
    }
    if (LoopFunction) {
        delete LoopFunction;
    }
    if(Lua!=0) {
        lua_close(Lua);
        Lua = 0;
    }
    for(std::list<DiaRuntimeScreen *>::iterator it = all_screens.begin(); it!=all_screens.end(); it++) {
        delete *it;
    }
}
