#ifndef _DIA_SCREEN_ITEM_DIGITS_H
#define _DIA_SCREEN_ITEM_DIGITS_H
#include <jansson.h>
#include <string>
#include "dia_screen_item.h"
#include "dia_all_items.h"
#include <SDL.h>
#include <SDL_image.h>

#define MAX_DIGITS 9

class DiaScreenItemDigits {
public:
    DiaIntPair position;
    DiaIntPair size;
    DiaFont font;
    DiaIntPair symbol_size;
    DiaNumber padding;
    DiaNumber length;
    DiaNumber value;
    SDL_Rect * OutputRectangles[MAX_DIGITS];
    int Init(DiaScreenItem * base_item,json_t * item_json);
    ~DiaScreenItemDigits();
    DiaScreenItemDigits();
};
#endif // _DIA_SCREEN_ITEM_DIGITS_H

int dia_screen_item_digits_display(DiaScreenItem * base_item, void * digits_ptr, DiaScreen * screen);
int dia_screen_item_digits_notify(DiaScreenItem * base_item, void * digits_ptr, std::string key);
