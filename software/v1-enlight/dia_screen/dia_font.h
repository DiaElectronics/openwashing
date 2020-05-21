#ifndef _DIA_FONT_H_
#define _DIA_FONT_H_

#include <jansson.h>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include "dia_all_items.h"

class DiaFont {
public:
    std::string name;
    DiaIntPair SymbolSize;
    SDL_Surface *FontImage;
    SDL_Surface *ScaledFontImage;
    SDL_Rect * SymbolRect[10];

    int Init(json_t * font_json);
    int Init(std::string folder, std::string newValue);
    DiaFont();
    ~DiaFont();
    int Scale(double xScale, double yScale);
};

#endif
