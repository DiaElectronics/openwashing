#include "dia_screen_config.h"
#include "dia_functions.h"
#include "dia_screen_item_image.h"
#include <chrono>

int DiaScreenConfig::Display(DiaScreen * screen) {
    Changed = 0;
    printf("Displaying screen '%s' ..........,,, \n", this->id.c_str());

    auto time_begin = std::chrono::high_resolution_clock::now();
    clickAreas.clear();
    

    for (auto it = items_list.begin(); it != items_list.end(); ++it) {
        DiaScreenItem * currentItem = *it;
        if (currentItem->display_ptr == 0) {
            printf("error: can't display object with empty display\n");
            return 1;
        }
        if (currentItem->specific_object_ptr == 0) {
            printf("error: can't display empty object\n");
            return 1;
        }

        if (currentItem->type == "image") {
            DiaScreenItemImage * currentItemImage = (DiaScreenItemImage *)(currentItem->specific_object_ptr);

            // Image is a clickable button
            if (currentItemImage->click_id.value != "0") {
                AreaItem button;
                button.X = currentItemImage->position.x;
                button.Y = currentItemImage->position.y;
                button.Width = currentItemImage->size.x;
                button.Height = currentItemImage->size.y;
                button.ID = currentItemImage->click_id.value;
                clickAreas.push_back(button);
            }
        }

        printf("--item '%s' of type '%s' --- \n", currentItem->id.c_str(), currentItem->type.c_str());
        if (currentItem->visible.value) {
            int err = currentItem->display_ptr(currentItem, currentItem->specific_object_ptr, screen);
            if (err!=0) {
                return err;
            }
        } else {
            printf("not visible!\n");
        }
    }
    
    screen->FlipFrame();
    auto time_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( time_end - time_begin ).count();
    printf("\n\nRENDERING DURATION: %ld\n\n\n", duration);
    return 0;
}
int DiaScreenConfig::Init(std::string folder, json_t * screen_json) {
    //printf("trying to initialize a screen config\n");
    Folder = folder;
    if(screen_json == 0) {
        printf("screen json is null\n");
        return 1;
    }

    //printf("1\n");
    json_t * id_json = json_object_get(screen_json, "id");
    if(!json_is_string(id_json)) {
        fprintf(stderr, "error: screen id is not a string\n");
        return 1;
    }
    std::string id_str=  json_string_value(id_json);
    //printf("2\n");
    id = id_str;
    //printf("screen id is found '%s' \n", id.c_str());

    json_t * src_json = json_object_get(screen_json, "src");
    if (src_json == 0) {
        printf("nil src for a screen\n");
        // parse screen from current file
        if(InitDetails(screen_json) != 0) return 1;
    } else {
        //printf("src is found\n");
        // parse screen from external source
        if (!json_is_string(src_json)) {
            printf("src is not a string");
            return 1;
        }
        
        const char * src_char = json_string_value(src_json);
        //std::string src = src_char;

        json_t * screen_implementation = dia_get_resource_json(folder.c_str(), src_char);
        if (screen_implementation == 0) {
            printf("error: can't load '%s' as json", src_char);
            return 1;
        }
        int err = InitDetails(screen_implementation);
        if (err != 0) {
            printf("init details failed \n");
            return 1;
        }

        json_decref(screen_implementation);
        printf("screen config init finished ...\n\n\n");
    }
    if(!json_is_string(src_json)) {
        fprintf(stderr, "error: screen src is not a string\n");
        return 1;
    }
    src = json_string_value(src_json);
    //printf("screen definition loaded: %s:%s\n", id.c_str(), src.c_str());
    return 0;
}

int DiaScreenConfig::AddItem(DiaScreenItem * item) {
    Changed = 1;
    if (item == 0) {
        printf("can't add nil element to the screen\n");
        return 1;
    }
    if(items_map.find("f") != items_map.end()) {
        printf("error: more than one element with ID=%s\n", item->id.c_str());
        return 1;
    }
    items_map.insert(std::pair<std::string, DiaScreenItem *>(item->id, item));
    items_list.push_back(items_map[item->id]);
    printf("screen element with '%s' id added\n", items_map[item->id]->id.c_str());
    return 0;
}

DiaScreenConfig::~DiaScreenConfig() {
    printf("Destroying screen config '%s'... \n", this->id.c_str());
    for (std::list<DiaScreenItem *>::iterator it=items_list.begin(); it != items_list.end(); ++it) {
        DiaScreenItem * currentItem = *it;
        printf("destroying item: '%s' \n", currentItem->id.c_str());
        delete currentItem;
    }
}

DiaScreenConfig::DiaScreenConfig() {
    Changed = 1;
}

int DiaScreenConfig::InitDetails(json_t *screen_json) {
    if (screen_json == 0) {
        printf("error: Init details nil json passed\n");
        return 1;
    }
    json_t *items_json = json_object_get(screen_json, "items");
    if (!json_is_array(items_json)) {
        printf("error: can't initialize display items\n");
        return 1;
    }
    for(unsigned int i = 0; i < json_array_size(items_json); i++) {
        //printf("loop by screen items %d\n", i);
        json_t * item_json = json_array_get(items_json, i);
        if (!json_is_object(item_json)) {
            printf("error: can't initialize one of display items %d\n", i+1);
            return 1;
        }
        DiaScreenItem * newItem = new DiaScreenItem(this);
        if(newItem->Init(item_json)) {
            printf("error happened while parsing specific item of a display\n");
            return 1;
        }
        if(AddItem(newItem)) return 1;
    }
    return 0;
}

int dia_screen_config_set_value_function (void * object, const char *element,
                                            const char * key, const char * value) {
    DiaScreenConfig * screen = (DiaScreenConfig *) object;

    DiaScreenItem * foundItem=screen->items_map[element];
    if (foundItem == 0) {
        printf("item is not found :( \n");
        return 1;
    }
    return foundItem->SetValue(key, value);
}

int dia_screen_display_screen (void * screen_object, void * screen_config) {
    DiaScreenConfig * screenConfig = (DiaScreenConfig *)screen_config;
    DiaScreen * screen = (DiaScreen *)screen_object;
    if (screenConfig->id == screen->LastDisplayed) {
        if (screenConfig->Changed) {
            screenConfig->Changed = 0;
            printf("disp:[%s]\n",screenConfig->id.c_str() );
            return screenConfig->Display(screen);
        }
        // if everything is the same we do not need to do anything
        return 1;
    }
    // else
    screen->LastDisplayed = screenConfig->id;
    screenConfig->Changed = 0;
    printf("real2:[%s]\n",screenConfig->id.c_str() );
    return screenConfig->Display(screen);
}
