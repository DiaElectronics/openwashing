#ifndef _DIA_FUNCTIONS_H
#define _DIA_FUNCTIONS_H

#include <string>
#include <jansson.h>
#include <SDL.h>

int base64_encode(const unsigned char *data, size_t input_length, char *encoded_data, size_t buf_size);
int base64_decode(const char *data, size_t input_length, char *decoded_data, size_t buf_size);
void build_decoding_table();
void base64_cleanup();
std::string dia_read_file(const char * file_name);
std::string dia_get_resource(const char * folder, const char *resource_name);
char * dia_int_to_str(int n, char * result);

SDL_Surface *dia_ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height);
json_t * dia_get_resource_json(const char * folder_name, const char * resource_name);

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 pixel);
Uint32 ReadPixel(SDL_Surface* surface, int x, int y);

#endif
