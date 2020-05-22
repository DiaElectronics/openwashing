#ifndef DIA_SCREEN_H
#define DIA_SCREEN_H
#include <stdio.h>

#define BPP 4
#define DEPTH 32

#define NO_ERROR 0
#define NULL_PASSED 1
#define IMG_ERROR 2
#define SDL_INIT_ERROR 3
#include "dia_screen.h"

DiaScreen::DiaScreen(int resX, int resY)
{
    //printf("SDLInitStart\n"); fflush(stdout);
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return;
    //printf("SDLInit finished properly, SDLShowCursor started\n"); fflush(stdout);
    SDL_ShowCursor(SDL_DISABLE);
    //printf("SDLShowCursor_2\n"); fflush(stdout);
	//printf("trying to set a video mode \n"); fflush(stdout);
	delay(100);
    	//if (!(Canvas = SDL_SetVideoMode(resX, resY, DEPTH, SDL_NOFRAME| SDL_HWSURFACE))) {      
	if (!(Canvas = SDL_SetVideoMode(resX, resY, DEPTH, SDL_NOFRAME|SDL_FULLSCREEN|SDL_HWSURFACE))) {
        printf("Cant set your videomode\n");
        SDL_Quit();
        InitializedOk = SDL_INIT_ERROR;
        return;
    }
    printf("video mode is set properly \n"); fflush(stdout);
	delay(100);
	InitializedOk = 1;
}

void DiaScreen_DrawPage1(DiaScreen * screen, int num) {
    printf("src1_s\n");
    //SDL_BlitSurface(screen->StationNumbers,
    //            screen->StationNumRect[(num)%10],
    //            screen->Canvas,
    //            screen->Dest);
    printf("src2_b2\n");
    SDL_Flip(screen->Canvas);
    printf("src2_b3\n");
}

void DiaScreen::FlipFrame() {
    printf("frame ... ");
    SDL_Flip(Canvas);
    printf(" ... flipped \n");
}
void DiaScreen::FillBackground(Uint8 r, Uint8 g, Uint8 b) {
	SDL_FillRect(Canvas, NULL, SDL_MapRGB(Canvas->format, r, g, b));
}

DiaScreen::~DiaScreen() {
    SDL_Quit();
}

#endif
