#include "dia_screen_item_image_array.h"
#include "dia_functions.h"

int DiaScreenItemImageArray::Init(DiaScreenItem *base_item, json_t * item_json) {
    if (item_json == 0) {
        printf("Item image array nil parameter\n");
        return 1;
    }
    printf("Image array init started \n");

    json_t * position_j = json_object_get(item_json, "position");
    if (base_item->SetValue("position", position_j)) return 1;

    json_t * size_j = json_object_get(item_json, "size");
    if (base_item->SetValue("size", size_j)) return 1;

    json_t * len_j = json_object_get(item_json, "length");
    if (base_item->SetValue("length", len_j)) return 1;

    json_t * to_show_j = json_object_get(item_json, "index");
    if (base_item->SetValue("index", to_show_j)) return 1;

    json_t * sources_j = json_object_get(item_json, "sources");
    if (!json_is_array(sources_j)) {
        fprintf(stderr, "error: sources is not an array\n");
        return 1;
    }

    int array_length = std::stoi(json_string_value(len_j));

    for (int i = 0; i < array_length; i++) {
        json_t * item = json_array_get(sources_j, i);
        json_t * src_j = json_object_get(item,"src");

        printf("Item path: %s\n", json_string_value(src_j));

        if (base_item->SetValue("src", src_j)) return 1;
    }
    
    return 0;
}

DiaScreenItemImageArray::DiaScreenItemImageArray() {
    for (int i = 0; i < MAX_PICTURES; i++) {
        Pictures[i] = 0;
        ScaledPictures[i] = 0;
    }
    OutputRectangle = (SDL_Rect *)malloc(sizeof(SDL_Rect));
    appendPos = 0;
}

void DiaScreenItemImageArray::AppendPicture(SDL_Surface * newPicture) {
    if (this->appendPos == MAX_PICTURES - 1) {
        printf("Image array OVERFLOW: can't append new image\n");
        return;
    }

    if (Pictures[this->appendPos] != 0) {
        SDL_FreeSurface(Pictures[this->appendPos]);
        Pictures[this->appendPos] = 0;
    }

    Pictures[this->appendPos] = newPicture;
    appendPos++;
}

DiaScreenItemImageArray::~DiaScreenItemImageArray() {
    for (int i = 0; i < MAX_PICTURES; i++) {
        if (Pictures[i] != 0) {
            SDL_FreeSurface(Pictures[i]);
            Pictures[i] = 0;
        }
    }
    if (OutputRectangle != 0) {
        free(OutputRectangle);
        OutputRectangle = 0;
    }
}


int dia_screen_item_image_array_display(DiaScreenItem * base_item, void * image_array_ptr, DiaScreen * screen) {
    printf("image array display function engaged\n");
    if (!base_item) {
        printf("error: nil base item\n");
        return 1;
    }

    if (!image_array_ptr) {
        printf("error: nil image_ptr\n");
        return 1;
    }

    DiaScreenItemImageArray * image_array = (DiaScreenItemImageArray *)image_array_ptr;

    printf("trying to display %d index, must be from 0 to %d\n", image_array->index.value, image_array->length.value - 1);
    int picture_to_show = image_array->index.value;
    if (picture_to_show < 0) {
        picture_to_show = 0;
    }
    int high_boundary = image_array->length.value - 1;
    if (picture_to_show > high_boundary) {
        picture_to_show = high_boundary;
    }
   
    SDL_Surface * currentPicture = image_array->Pictures[picture_to_show];

    SDL_BlitSurface(currentPicture,
                NULL,
                screen->Canvas,
                image_array->OutputRectangle);

    printf("img displayed at (%d, %d) size (%d, %d) \n",
    image_array->OutputRectangle->x,
    image_array->OutputRectangle->y,
    image_array->OutputRectangle->w,
    image_array->OutputRectangle->h);
    
    return 0;
}

int dia_screen_item_image_array_notify(DiaScreenItem * base_item, void * image_array_ptr, std::string key) {
    //printf("Notification in Image Array for '%s' key\n", key.c_str());

    int error = 0;
    std::string value = base_item->GetValue(key, &error);

    //printf("GetValue: %s\n", value.c_str());

    if (error != 0) {
        printf("notification on non-existing key '%s'\n", key.c_str());
        return 1;
    }

    DiaScreenItemImageArray *obj =  (DiaScreenItemImageArray *)image_array_ptr;

    if (key.compare("position") == 0) {
        obj->position.Init(value);
        obj->OutputRectangle->x = obj->position.x;
        obj->OutputRectangle->y = obj->position.y;
    } else
    if (key.compare("size") == 0) {
        obj->size.Init(value);
        obj->OutputRectangle->w = obj->size.x;
        obj->OutputRectangle->h = obj->size.y;
    } else
    if (key.compare("length") == 0) {
        obj->length.Init(value);
    } else 
    if (key.compare("index") == 0) {
        obj->index.Init(value);
    } else
    if (key.compare("src") == 0) {
        std::string full_name = base_item->Parent->Folder;

        full_name += "/";
        full_name += value;

        SDL_Surface *newImg = IMG_Load(full_name.c_str());

        if (!newImg) {
            printf("error: IMG_Load: %s\n", IMG_GetError());
            printf("%s error\n", full_name.c_str());
            return 1;
        }
        obj->AppendPicture(newImg);
	} else {
        printf("unknown key for image object: '%s' \n", key.c_str());
        return 1;
    }

    return 0;
}
