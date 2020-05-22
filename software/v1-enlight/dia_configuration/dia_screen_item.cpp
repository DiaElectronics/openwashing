#include "dia_screen_item.h"
#include "dia_screen_item_digits.h"
#include "dia_screen_item_image.h"
#include "dia_screen_item_image_array.h"

DiaScreenItem::DiaScreenItem(DiaScreenConfig * newParent) {
    display_ptr = 0;
    notify_ptr = 0;
    specific_object_ptr = 0;
    Parent = newParent;
}

int DiaScreenItem::Init(json_t * screen_item_json) {
    // we need to parse ID and TYPE
    //printf("item init triggered \n" );
    json_t * id_json = json_object_get(screen_item_json, "id");
    if (!json_is_string(id_json)) {
        printf("error: id of screen item is not a string\n");
        return 1;
    }
    id = json_string_value(id_json);
    //printf("item init id='%s' \n", id.c_str());
    ///////////////////////////////////
    json_t * type_json = json_object_get(screen_item_json, "type");
    if (!json_is_string(type_json)) {
        printf("error: type of screen item is not a string\n");
        return 1;
    }
    type = json_string_value(type_json);
    //printf("item init type='%s'\n", type.c_str());

    json_t * visible_json = json_object_get(screen_item_json, "visible");
    if (visible_json == 0) {
        SetValue("visible", "true");
    } else {
        if (!json_is_string(visible_json)) {
            printf("error: visible of screen item is not a string\n");
            return 1;
        }
        std::string visibleString = json_string_value(visible_json);
        SetValue("visible", visibleString);
    }

    if(type.compare("digits")==0) {
        DiaScreenItemDigits * digits = new DiaScreenItemDigits();


        this->specific_object_ptr = digits;
        this->notify_ptr = dia_screen_item_digits_notify;
        this->display_ptr = dia_screen_item_digits_display;

        digits->Init(this, screen_item_json);
    } else if(type.compare("image")==0) {
        //printf("image object found...\n");
        DiaScreenItemImage * image = new DiaScreenItemImage();


        this->specific_object_ptr = image;
        this->notify_ptr = dia_screen_item_image_notify;
        this->display_ptr = dia_screen_item_image_display;

        image->Init(this, screen_item_json);
    } else if (type.compare("image_array") == 0) {
        printf("Image Array found...\n");
        
        DiaScreenItemImageArray * image_array = new DiaScreenItemImageArray();

        this->specific_object_ptr = image_array;
        this->notify_ptr = dia_screen_item_image_array_notify;
        this->display_ptr = dia_screen_item_image_array_display;

        image_array->Init(this, screen_item_json);

    } else {
        printf("unknown type:[%s]\n",type.c_str());
        return 1;
    }

    return 0;
}

std::string DiaScreenItem::GetValue(std::string key, int * error) {
    if (items.find(key) == items.end()) {
        printf("item [%s] is not found\n", key.c_str());
        if (error!=0) *error = 1;
        return "";
    }
    if(error!=0) *error = 0;
    std::string res = items[key];
    //printf("value unpacked ='%s':'%s'\n", key.c_str(), res.c_str());
    return res;
}
int DiaScreenItem::SetValue(std::string key, json_t * value) {
    if (value == 0) {
        printf("cant set 0 value to an item\n");
        return 1;
    }
    if (!json_is_string(value)) {
        printf("for now just string values allowed for screen items, sorry\n");
        return 1;
    }
    std::string value_str = json_string_value(value);
    return SetValue(key, value_str);
}

int DiaScreenItem::SetValue(std::string key, std::string value) {
    //printf("Set value for string started: key=%s, value=%s\n", key.c_str(), value.c_str());
    // Let's check common values first
    if (key.compare("visible")==0) {
        int oldVisible = visible.value;
        int err = visible.Init(value);
        if (err) return err;
        if (oldVisible != visible.value) {
            Parent->Changed = 1;
        }
        return 0;
    }

    if (items.find(key) == items.end()) {
        //printf("new item ... \n");
        Parent->Changed = true;

        items[key] = value;
        if(notify_ptr!=0) {
            return notify_ptr(this, specific_object_ptr, key);
        }
    } else {
        
        std::string oldValue = items[key];
        items[key] = value;
        if (oldValue.compare(value)!=0) {
            Parent->Changed = 1;
            if(notify_ptr!=0) {
                return notify_ptr(this, specific_object_ptr, key);
            } else {
                printf("can't notify element, warning\n");
            }
        }
    }

    return 0;
}

DiaScreenItem::~DiaScreenItem() {
    printf("~DiaScreenItem();\n");
    if(type.compare("digits")==0 ) {
        printf("digits object to be destroyed...\n");
        if(specific_object_ptr!=0) {
            DiaScreenItemDigits * digits = (DiaScreenItemDigits *) this->specific_object_ptr;
            delete digits;
            specific_object_ptr = 0;
            printf("digits deleted \n");
        } else {
            printf("digits can't be deleted \n");
        }
    } else if(type.compare("image")==0 ) {
        printf("image object to be destroyed...\n");
        if(specific_object_ptr!=0) {
            DiaScreenItemImage * image = (DiaScreenItemImage *) this->specific_object_ptr;
            delete image;
            specific_object_ptr = 0;
            printf("image deleted \n");
        } else {
            printf("image can't be deleted \n");
        }
    } else {
        printf("error, can't destroy type '%s'\n", type.c_str() );
    }
}
