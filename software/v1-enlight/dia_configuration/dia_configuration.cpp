#include "dia_configuration.h"
#include "dia_functions.h"
#include "dia_storage_interface.h"
#include <string.h>
#include <stdlib.h>

int DiaConfiguration::InitFromFile() {
    //printf("loading config from resource: '%s' / '%s'\n", _Folder.c_str(), DIA_DEFAULT_FIRMWARE_FILENAME);
    std::string resource = dia_get_resource(_Folder.c_str(), DIA_DEFAULT_FIRMWARE_FILENAME);
    return InitFromString(resource.c_str());
}

int DiaConfiguration::RunCommand(std::string command) {
    return 0;
}

int DiaConfiguration::Setup() {
    return GetRuntime()->Setup();
}

int DiaConfiguration::Loop() {
    return GetRuntime()->Loop();
}

int DiaConfiguration::InitFromString(const char * configuration_json) {
    json_t *root;
    json_error_t error;
    root = json_loads(configuration_json, 0, &error);

    if(!root) {
        fprintf(stderr, "json error: on line %d: %s\n", error.line, error.text);
        printf("json:\n%s\n\n", configuration_json);
        return 1;
    }

    //printf("json parsed\n");
    int res = InitFromJson(root);
    //printf("configuration parsed\n");
    json_decref(root);
    return res;
}

int DiaConfiguration::Init() {
    int err = InitFromFile();
    if(err) {
        return err;
    }
    err = LoadConfig();
    if(err) {
        return err;
    }
    _Gpio =0;
    if (!err) {
        int hideMouse = 0;
        int fullScreen = 0;
        #ifdef USE_GPIO
        hideMouse = 1;
        fullScreen = 1;
        #endif
        #ifdef DEBUG
        hideMouse = 0;
        fullScreen = 0;
        #endif
        _Screen = new DiaScreen(GetResX(), GetResY(), hideMouse, fullScreen);
        if (_Screen->InitializedOk!=1) {
            return _Screen->InitializedOk;
        }
        _Gpio = 0;
        #ifdef USE_GPIO
        _Gpio = new DiaGpio(GetButtonsNumber(), GetRelaysNumber(), GetStorage());
        if (!_Gpio->InitializedOk) {
            printf("ERROR: GPIO INIT");
            return 1;
        }

        // Let's copy programs
        std::map<int, DiaProgram *>::iterator it;
        int programNum = 0;
        for(it=_Programs.begin();it!=_Programs.end();it++) {
            programNum++;
            DiaProgram * curProgram = it->second;
            for(int i=0;i<PIN_COUNT_CONFIG;i++) {
                if (curProgram->Relays.RelayNum[i]>0) {
                    int id = curProgram->Relays.RelayNum[i];
                    int ontime = curProgram->Relays.OnTime[i];
                    int offtime = curProgram->Relays.OffTime[i];
                    _Gpio->Programs[curProgram->ButtonID].InitRelay(id, ontime, offtime);
                }
                if (curProgram->PreflightRelays.RelayNum[i]>0) {
                    int id = curProgram->PreflightRelays.RelayNum[i];
                    int ontime = curProgram->PreflightRelays.OnTime[i];
                    int offtime = curProgram->PreflightRelays.OffTime[i];
                    _Gpio->PreflightPrograms[curProgram->ButtonID].InitRelay(id, ontime, offtime);
                }
            }
        }
        #endif
    }
    return err;
}

DiaConfiguration::DiaConfiguration(std::string folder, DiaNetwork *newNet) {
    _Name = "";
    _Folder = folder;
    _ResX = 0;
    _ResY = 0;
    _ButtonsNumber = 0;
    _RelaysNumber = 0;
    _PreflightSec = 0;
    _ServerRelayBoard = 0;

    // Must be rearranged
    registry = new DiaRuntimeRegistry(newNet);
    _Runtime = new DiaRuntime(registry);
    _Gpio = 0;
    _Screen = 0; // use Init();
    _svcWeather = new DiaRuntimeSvcWeather(newNet);

    _Storage = CreateEmptyInterface();
    _Net = newNet;
}
int DiaConfiguration::InitFromJson(json_t * configuration_json) {
    if (configuration_json == 0) {
         return 1;
    }

    // Let's unpack Name
    json_t * name_json = json_object_get(configuration_json, "name");
    if(!json_is_string(name_json)) {
        fprintf(stderr, "error: configuration name is not a string\n");
        return 1;
    }

    _Name = json_string_value(name_json);
    //printf("Configuration's name is parsed: ");
    //printf("%s\n", _Name.c_str());

    // Let's unpack Screens
    json_t * screens_json = json_object_get(configuration_json, "screens");
    if(!json_is_array(screens_json)) {
        fprintf(stderr, "error: screens is not an array\n");
        return 1;
    }

    json_t *resolution_json = json_object_get(configuration_json, "resolution");
    if(!json_is_string(resolution_json)) {
        fprintf(stderr, "error: resolution is not a string\n");
        return 1;
    }

    std::string _Resolution = json_string_value(resolution_json);
    int _ResN = _Resolution.length();
    _ResX = 0;
    int resolutionCursor;
    for (resolutionCursor=0; resolutionCursor < _ResN; resolutionCursor++) {
        char key = _Resolution[resolutionCursor];
        if (key>='0' && key<='9') {
            _ResX*=10;
            _ResX+=key-'0';
        } else {
            break;
        }
    }
    _ResY =0;
    resolutionCursor++;
     for (; resolutionCursor< _ResN; resolutionCursor++) {
        char key = _Resolution[resolutionCursor];
        if (key>='0' && key<='9') {
            _ResY*=10;
            _ResY+=key-'0';
        } else {
            printf("resolution parsing error: [%s], must by like [848x480]\n", _Resolution.c_str());
        }
    }
    //printf("resolution:[%s], parsed ad [%d]*[%d]\n", _Resolution.c_str(), _ResX, _ResY);

    _NeedToRotateTouchScreen = 0;
    json_t *touch_rotate_json = json_object_get(configuration_json, "touch_rotate");
    if(json_is_string(touch_rotate_json)) {
        _NeedToRotateTouchScreen = 180;
    }

    // Let's unpack buttons #
    json_t *buttons_json = json_object_get(configuration_json, "buttons");
    if(!json_is_integer(buttons_json)) {
        fprintf(stderr, "error: buttons is not an integer\n");
        return 1;
    }

    _ButtonsNumber = json_integer_value(buttons_json);


    // Let's check if we can use the last button as a pulse coin

    _LastButtonPulse = 0;
    json_t *last_button_pulse_json = json_object_get(configuration_json, "last_button_pulse");
    if (last_button_pulse_json) {
        if(json_is_boolean(last_button_pulse_json)) {
            _LastButtonPulse = json_boolean_value(last_button_pulse_json);
        } else if (json_is_integer(last_button_pulse_json)) {
            _LastButtonPulse = json_integer_value(last_button_pulse_json);
        }
    }

    // Let's unpack relays #
    json_t *relays_json = json_object_get(configuration_json, "relays");
    if(!json_is_integer(relays_json)) {
        fprintf(stderr, "error: relays is not a integer\n");
        return 1;
    }
    _RelaysNumber = json_integer_value(relays_json);

    for(unsigned int i = 0; i < json_array_size(screens_json); i++) {
        //printf("screen loop \n");
        json_t * screen_json = json_array_get(screens_json, i);
        if(!json_is_object(screen_json)) {
            fprintf(stderr, "error: screen %d is not an object\n", i + 1);
            return 1;
        }

        DiaScreenConfig * screen_parsed = new DiaScreenConfig();
        json_t * id_json = json_object_get(screen_json, "id");
        if(!json_is_string(id_json)) {
            fprintf(stderr, "error: screen id is not a string\n");
            return 1;
        }

        std::string id = json_string_value(id_json);
        ScreenConfigs.insert(std::pair<std::string, DiaScreenConfig *>(id, screen_parsed));
        ScreenConfigs[id]->Init(_Folder, screen_json);
    }

    json_t * script_json = json_object_get(configuration_json, "script");
    json_t * include_json = json_object_get(configuration_json, "include");
    return GetRuntime()->Init(_Folder, script_json, include_json);
}

int DiaConfiguration::LoadConfig() {
    std::string answer;
    int err = _Net->GetStationConig(answer);
    if (err !=0) {
        fprintf(stderr, "error: load config\n");
        return 1;
    }
    json_error_t error;
    json_t * configuration_json;
    configuration_json = json_loads(answer.c_str(), 0, &error);
    if (!configuration_json) {
        printf("Error in LoadConfig: %d: %s\n", error.line, error.text );
        return 1;
    }

    if(!json_is_object(configuration_json)) {
	    printf("LoadConfig not a JSON\n");
                return 1;
        }

    json_t *preflight_json = json_object_get(configuration_json, "preflightSec");
    if(json_is_integer(preflight_json)) {
        _PreflightSec = json_integer_value(preflight_json);
    }
    json_t *relay_board_json = json_object_get(configuration_json, "relayBoard");
    if(json_is_string(relay_board_json)) {
        std::string board = json_string_value(relay_board_json);
        printf("LoadConfig relay board %s\n", board.c_str());
        if (board == "danBoard") {
            _ServerRelayBoard = 1;
        }
    }

    // Let's unpack programs
    json_t *programs_json = json_object_get(configuration_json, "programs");
    if(!json_is_array(programs_json)) {
        fprintf(stderr, "error: programs is not an array\n");
        return 1;
    }
    for (unsigned int i=0;i< json_array_size(programs_json); i++) {
        //printf("programs reading loop\n");
        json_t * program_json = json_array_get(programs_json, i);
        if(!json_is_object(program_json)) {
            fprintf(stderr, "error: program %d is not an object\n", i + 1);
            return 1;
        }
        // XXX rebuild
        DiaProgram * program = new DiaProgram(program_json);
        if(!program->_InitializedOk) {
            printf("Something's wrong with the program");
            return 1;
        }
        this->_Programs[program->ButtonID] = program;
    }
        for (int i=1;i<= _ButtonsNumber; i++) {
            if (!this->_Programs[i]) {
            fprintf(stderr, "error: LoadConfig buttonID %d not found\n", i);
            return 1;
            }
        }
    return 0;
}

DiaConfiguration::~DiaConfiguration() {
    printf("Destroying configuration ... \n");

    std::map<std::string, DiaScreenConfig *>::iterator it;
    for (it=ScreenConfigs.begin(); it!=ScreenConfigs.end(); it++) {
        DiaScreenConfig * curConfig = it->second;
        if(curConfig!=0) {
            delete curConfig;
        }
    }
    delete _Runtime;
    if (_Screen) {
        delete _Screen;
    }
    if(_Gpio) {
        delete _Gpio;
    }
    if(registry) {
        delete registry;
    }
    if(_svcWeather) {
        delete _svcWeather;
    }
}
