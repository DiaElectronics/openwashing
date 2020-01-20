#include "dia_font.h"
#include <string>
#include "dia_functions.h"

int DiaFont::Init(json_t * font_json) {
    if (font_json == 0) {
        printf("nil parameter passed to font init\n");
        return 1;
    }
    if (!json_is_string(font_json)) {
        printf("font description is not a string, for now it must be string only");
    }
    name = json_string_value(font_json);

    return 0;
}

int DiaFont::Init(std::string folder, std::string newValue) {
    std::string full_name = folder;
        full_name+="/numbers.png";

    FontImage=IMG_Load(full_name.c_str());
    if(!FontImage) {
        printf("error: IMG_Load: %s\n", IMG_GetError());
        printf("%s error\n", full_name.c_str());
        return 1;
	}
    name = newValue;
    return 0;
}

DiaFont::DiaFont() {
    FontImage = 0;
    ScaledFontImage = 0;
    SymbolSize.x = 235;
    SymbolSize.y = 405;
    for(int i=0;i<10;i++) {
        SymbolRect[i] = (SDL_Rect *) malloc(sizeof(SDL_Rect));
        SymbolRect[i]->x = i * 265;
        SymbolRect[i]->y = 0;
        SymbolRect[i]->w = 235; //All symbols have about 30 pixels of wide space on the right side
        SymbolRect[i]->h = 405;
    }

}

int DiaFont::Scale(double xScale, double yScale) {
    printf("font scale (%f,%f)\n",xScale, yScale);
    int newWidth = (int)(2650.0 * xScale);
    int newHeight = (int)(405.0 * yScale);
    ScaledFontImage = dia_ScaleSurface(FontImage, newWidth, newHeight);

    //here is a code for scaling the surface

    int newSymbolWidth = (int)(235.0 * xScale);
    for(int i=0;i<10;i++) {
        double xOffset = (double)(i * 265);
        SymbolRect[i]->x = (int)(xOffset * xScale);
        SymbolRect[i]->w = newSymbolWidth; //All symbols have about 30 pixels of wide space on the right side
        SymbolRect[i]->h = 405;
    }

    return 0;
}

DiaFont::~DiaFont() {
    if(FontImage != 0) {
        SDL_FreeSurface(FontImage);
        FontImage = 0;
    }

    if(ScaledFontImage != 0) {
        SDL_FreeSurface(ScaledFontImage);
    }

    for(int i=0;i<10;i++) {
        if (SymbolRect[i]!=0) {
            free(SymbolRect[i]);
            SymbolRect[i] = 0;
        } else {
            printf("err: attempt to destroy already destroyed font\n");
        }
    }
}
