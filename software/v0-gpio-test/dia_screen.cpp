#ifndef DIA_SCREEN_H
#define DIA_SCREEN_H
#include <stdio.h>


#define WIDTH 848
#define HEIGHT 480
#define BPP 4
#define DEPTH 32

#define NO_ERROR 0
#define NULL_PASSED 1
#define IMG_ERROR 2
#define SDL_INIT_ERROR 3
#include "dia_screen.h"

DiaScreen::DiaScreen()
{
    InitializedOk = 0;
    Number = 1024;
	NumbersImage=IMG_Load("numbers.png");
	if(!NumbersImage) {
		printf("IMG_Load: %s\n", IMG_GetError());
		InitializedOk  = IMG_ERROR;
		return ;
	}

	if (!(Canvas = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_NOFRAME|SDL_FULLSCREEN|SDL_HWSURFACE)))
    {
        SDL_Quit();
        InitializedOk = SDL_INIT_ERROR;
        return;
    }

    for(int i=0;i<10;i++)
    {
		//the position of numbers in the file 1234567890, so we must shift
		int j=i-1;
		if (j<0)
		{
			j=9;
		}
		numrect[i] = (SDL_Rect *)malloc(sizeof(SDL_Rect));
		numrect[i]->x = 267 * j;
		numrect[i]->y = 0;
		numrect[i]->w = 255;
		numrect[i]->h = 404;
	}
	for (int i=0;i<4;i++)
	{
		outputrect[i] = (SDL_Rect *)malloc(sizeof(SDL_Rect));
		outputrect[i]->x = 20 + i * 280;
		outputrect[i]->y = 40;
		outputrect[i]->w = 256;
		outputrect[i]->h = 404;
	}
	InitializedOk = 1;
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return;
    SDL_ShowCursor(SDL_DISABLE);
}

void DiaScreen_Setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;

    colour = SDL_MapRGB( screen->format, r, g, b );

    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}

void DiaScreen_DisplayNumber(DiaScreen * screen, int num)
{
	char nums[4];

	nums[0] = (num/100)%10;
	nums[1] = (num/10)%10;
	nums[2] = (num)%10;

	for(int j=0;j<3;j++)
	{
		SDL_BlitSurface(screen->NumbersImage,
					screen->numrect[nums[j]],
					screen->Canvas,
					screen->outputrect[j]);
	}

}

void DiaScreen_DrawProcedure(DiaScreen * screen)
{
	SDL_FillRect(screen->Canvas, NULL, SDL_MapRGB(screen->Canvas->format, 255, 255, 255));
	DiaScreen_DisplayNumber(screen, screen->Number);
}

void DiaScreen_DrawScreen(DiaScreen * screen)
{
    if(SDL_MUSTLOCK(screen->Canvas))
    {
        if(SDL_LockSurface(screen->Canvas) < 0) return;
    }
    DiaScreen_DrawProcedure(screen);
    if(SDL_MUSTLOCK(screen->Canvas)) SDL_UnlockSurface(screen->Canvas);
    SDL_Flip(screen->Canvas);
}
DiaScreen::~DiaScreen()
{
    SDL_Quit();
}

#endif
