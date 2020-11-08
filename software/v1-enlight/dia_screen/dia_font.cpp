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
    full_name += "/pic/fonts/" + newValue + ".png";

    printf("Dia Font Init - loading font from file: %s\n", full_name.c_str());
    FontImage = IMG_Load(full_name.c_str());
    if (!FontImage) {
        printf("error: IMG_Load: %s\n", IMG_GetError());
        printf("%s error\n", full_name.c_str());
        return 1;
	}
    name = newValue;

    return 0;
}

int DiaFont::InitSymbols(int is_vertical) {
    // This code is adapted for vertical HD screen
    if (is_vertical) {
        SymbolSize.x = FontImage->w;
        SymbolSize.y = FontImage->h / 10.0;
        printf("DIA FONT: W = %d, H = %d\n", FontImage->w, FontImage->h);
        printf("DIA FONT: Symbol Size - X = %d, Y = %d\n", SymbolSize.x, SymbolSize.y);

        for (int i = 0; i < 10; i++) {
            SymbolRect[i] = (SDL_Rect *) malloc(sizeof(SDL_Rect));

            SymbolRect[i]-> x = 0;
            SymbolRect[i]-> y = i * SymbolSize.y;
            SymbolRect[i]-> w = SymbolSize.x;
            SymbolRect[i]-> h = SymbolSize.y;
        }
    } else {
        // This code is for legacy horizontal screen
        int symbolWidth = FontImage->w / 10;
        int symbolHeight = FontImage->h;
        SymbolSize.x = symbolWidth;
        SymbolSize.y = symbolHeight;
        for(int i=0;i<10;i++) {
            SymbolRect[i] = (SDL_Rect *) malloc(sizeof(SDL_Rect));
            SymbolRect[i]->x = i * symbolWidth;
            SymbolRect[i]->y = 0;
            SymbolRect[i]->w = symbolWidth;
            SymbolRect[i]->h = symbolHeight;
        }
    }
    
    return 0;
}

DiaFont::DiaFont() {
    FontImage = 0;
    ScaledFontImage = 0;
}

int DiaFont::Scale(double xScale, double yScale, int is_vertical) {
    // This code is adapted for vertical HD screen
    if (is_vertical) {
        printf("Dia Font Scale (%f,%f)\n", xScale, yScale);

        int newWidth = (int)((FontImage->w) * xScale);
        int newHeight = (int)((FontImage->h) * yScale);
        ScaledFontImage = dia_ScaleSurface(FontImage, newWidth, newHeight);

        int newSymbolHeight = (int)(SymbolSize.y * yScale);
    
        for (int i = 0; i < 10; i++) {
            double yOffset = (double)(i * SymbolSize.y);
            SymbolRect[i]->y = (int)(yOffset * yScale);
            SymbolRect[i]->w = SymbolSize.x; 
            SymbolRect[i]->h = newSymbolHeight;
        }

    // This code is for legacy horizontal screen
    } else {
        int newWidth = (int)(FontImage->w * xScale);
        int newHeight = (int)(FontImage->h * yScale);
        ScaledFontImage = dia_ScaleSurface(FontImage, newWidth, newHeight);

        int newSymbolWidth = (int)(SymbolSize.x * xScale);
        for(int i=0;i<10;i++) {
            double xOffset = (double)(i * SymbolSize.x);
            SymbolRect[i]->x = (int)(xOffset * xScale);
            SymbolRect[i]->w = newSymbolWidth;
            SymbolRect[i]->h = newHeight;
        }
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

    for (int i = 0; i < 10; i++) {
        if (SymbolRect[i] != 0) {
            free(SymbolRect[i]);
            SymbolRect[i] = 0;
        } else {
            printf("err: attempt to destroy already destroyed font\n");
        }
    }
}
