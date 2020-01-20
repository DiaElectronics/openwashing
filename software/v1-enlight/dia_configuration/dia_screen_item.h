#ifndef _DIA_SCREEN_ITEM_H
#define _DIA_SCREEN_ITEM_H
#include <string>
#include <jansson.h>
#include <map>
#include "dia_screen.h"
#include "dia_screen_config.h"
#include "dia_all_items.h"

class DiaScreenItem {
private:
    std::map<std::string, std::string> items;
public:
    std::string id;
    std::string type;
    DiaBoolean visible;
    DiaScreenConfig *Parent;


    // virtual function to display an element

    //display_ptr needs to draw specific element on the screen
    int (*display_ptr)(DiaScreenItem *, void *, DiaScreen *);

    //notify_ptr needs to notify the element about element was changed
    int (*notify_ptr)(DiaScreenItem *, void *, std::string);

    // the object pointer to the specific item;
    void * specific_object_ptr;

    // end of virtual functions block
    DiaScreenItem(DiaScreenConfig * newParent);
    ~DiaScreenItem();
    int Init(json_t * screen_item_json);
    std::string GetValue(std::string key, int * error);
    int SetValue(std::string key, std::string value);
    int SetValue(std::string key, json_t * value);
};


#endif

