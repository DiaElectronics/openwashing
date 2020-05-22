#ifndef _DIA_SCREEN_ITEM_IMAGE_H
#define _DIA_SCREEN_ITEM_IMAGE_H
#include <jansson.h>
#include <string>
#include "dia_screen_item.h"
#include "dia_all_items.h"
#include <SDL.h>
#include <SDL_image.h>

class DiaScreenItemImage {
public:
    DiaIntPair position;
    DiaIntPair size;
    DiaString src;
    DiaString click_id;

    SDL_Rect * OutputRectangle;
    SDL_Surface * Picture;
    SDL_Surface * ScaledPicture;
    int Init(DiaScreenItem * base_item,json_t * item_json);
    void SetPicture(SDL_Surface * newPicture);
    void SetScaledPicture(SDL_Surface * newPicture);
    void Rescale();
    ~DiaScreenItemImage();
    DiaScreenItemImage();
};
#endif // _DIA_SCREEN_ITEM_IMAGE_H

int dia_screen_item_image_display(DiaScreenItem * base_item, void * image_ptr, DiaScreen * screen);
int dia_screen_item_image_notify(DiaScreenItem * base_item, void * image_ptr, std::string key);
