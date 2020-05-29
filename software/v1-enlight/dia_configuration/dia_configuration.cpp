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
    _Gpio =0;
    if (!err) {
        int hideMouse = 0;
        int fullScreen = 0;
        #ifdef USE_GPIO
        hideMouse = 1;
        fullScreen = 1;
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
        std::map<std::string, DiaProgram *>::iterator it;
        int programNum = 0;
        for(it=_Programs.begin();it!=_Programs.end();it++) {
            programNum++;
            DiaProgram * curProgram = it->second;
            for(int i=0;i<PIN_COUNT_CONFIG;i++) {
                if (curProgram->Config.RelayNum[i]>0) {
                    int id = curProgram->Config.RelayNum[i];
                    int ontime = curProgram->Config.OnTime[i];
                    int offtime = curProgram->Config.OffTime[i];
                    _Gpio->Programs[programNum].InitRelay(id, ontime, offtime);
                }
            }

            printf("PROGRAM CONFIG UPDATED:[%s]->[%d]\n", it->first.c_str(), programNum);
            //_Gpio->Programs[1].OnTime[1] = 250;
            //_Gpio->Programs[1].OffTime[1] = 250;
            //_Gpio->Programs[1].InitRelay(3,1000, 1000);
            printf("program 1-1: %ld,%ld\n",
                _Gpio->Programs[1].OnTime[1],
                _Gpio->Programs[1].OffTime[1]);
            printf("program 1-3: %ld,%ld\n",
                _Gpio->Programs[1].OnTime[3],
                _Gpio->Programs[1].OffTime[3]);


            _Gpio->_ProgramMapping[it->first] = programNum;
        }
        #endif
    }
    return err;
}

DiaConfiguration::DiaConfiguration(std::string folder) {
    _Name = "";
    _Folder = folder;
    _ResX = 0;
    _ResY = 0;
    _ButtonsNumber = 0;
    _RelaysNumber = 0;
    _Runtime = new DiaRuntime();
    _Gpio = 0;
    _Screen = 0; // use Init();

    // let's init income
    _Income.totalIncomeCoins = 0;
    _Income.totalIncomeBanknotes = 0;
    _Income.totalIncomeElectron = 0;
    _Income.totalIncomeService = 0;
    _Income.carsTotal = 0;

    _Storage = CreateEmptyInterface();
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

    // Let's unpack programs
    json_t *programs_json = json_object_get(configuration_json, "programs");
    if(!json_is_array(programs_json)) {
        fprintf(stderr, "error: programs is not an array\n");
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

    // Let's unpack buttons #
    json_t *buttons_json = json_object_get(configuration_json, "buttons");
    if(!json_is_integer(buttons_json)) {
        fprintf(stderr, "error: buttons is not an integer\n");
        return 1;
    }

    _ButtonsNumber = json_integer_value(buttons_json);

    // Let's unpack relays #
    json_t *relays_json = json_object_get(configuration_json, "relays");
    if(!json_is_integer(relays_json)) {
        fprintf(stderr, "error: relays is not a integer\n");
        return 1;
    }
    _RelaysNumber = json_integer_value(relays_json);

    // Let's unpack the endpoint
    json_t *endpoint_json = json_object_get(configuration_json, "endpoint");
    if(!json_is_string(endpoint_json)) {
        fprintf(stderr, "error: endpoint is not a string\n");
        return 1;
    }

    _Endpoint = json_string_value(endpoint_json);

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

    for (unsigned int i=0;i< json_array_size(programs_json); i++) {
        //printf("programs reading loop\n");
        json_t * program_json = json_array_get(programs_json, i);
        if(!json_is_object(program_json)) {
            fprintf(stderr, "error: program %d is not an object\n", i + 1);
            return 1;
        }
        // XXX rebuild
        DiaProgram * program = new DiaProgram(program_json, _Folder);
        if(!program->_InitializedOk) {
            printf("Something's wrong with the program");
            return 1;
        }
        this->_Programs[program->ID] = program;
    }
    json_t * script_json = json_object_get(configuration_json, "script");
    return GetRuntime()->Init(_Folder, script_json);
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
}
