#ifndef dia_screen_sdl1
#define dia_screen_sdl1

#include <SDL.h>
#include <SDL_image.h>

class DiaScreen
{
    public:
    DiaScreen();
    ~DiaScreen();
    int InitializedOk;
    int Number;
	SDL_Surface *NumbersImage;
	SDL_Rect* numrect[10];

	SDL_Surface * Canvas;
	SDL_Rect* outputrect[4];
};

void DiaScreen_Setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b);
void DiaScreen_DisplayNumber(DiaScreen * screen, int num);
void DiaScreen_DrawProcedure(DiaScreen * screen);
void DiaScreen_DrawScreen(DiaScreen * screen);

#endif
