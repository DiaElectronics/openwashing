#ifndef dia_screen_sdl1
#define dia_screen_sdl1

#include <SDL.h>
#include <SDL_image.h>
#include "global_settings.h"
#include <wiringPi.h>
#include <string>
class DiaScreen
{
    public:
    DiaScreen(int resX, int resY);
    ~DiaScreen();
    int InitializedOk;
    int Number;
    std::string LastDisplayed;
	SDL_Surface * Canvas;
	void FillBackground(Uint8 r, Uint8 g, Uint8 b);
	void FlipFrame();
};

#endif
