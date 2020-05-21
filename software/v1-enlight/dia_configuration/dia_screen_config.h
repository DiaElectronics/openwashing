#ifndef dia_screen_config_h
#define dia_screen_config_h

#include <map>
#include <list>

#include <string>
#include <jansson.h>
#include "dia_screen_item.h"
#include "dia_screen.h"
#include "dia_all_items.h"

class DiaScreenConfig {
public:
    std::string id;
    std::string src;
    std::string Folder;
    std::string background;
    int Changed;

    std::list<DiaScreenItem *> items_list;
    std::map<std::string, DiaScreenItem *> items_map;

    int Init(std::string folder, json_t * screen_json); //Will never be a virtual function
    int InitDetails(json_t *screen_json);
    int AddItem(DiaScreenItem * item);
    int Display(DiaScreen * screen);
    ~DiaScreenConfig();
    DiaScreenConfig();
};

int dia_screen_config_set_value_function (void * object, const char *element, const char * key, const char * value);
int dia_screen_display_screen (void * screen_object, void * screen_config);

#endif
