#include <stdint.h>
#include <stdlib.h>

#include "dia_functions.h"
#include "string.h"
#include <unistd.h>
#include <cmath>

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char decoding_table[256];
static int initialized_decoding_table = 0;

static int mod_table[] = {0, 2, 1};


int base64_encode(const unsigned char *data, size_t input_length, char *encoded_data, size_t buf_size) {
    int output_length = 4 * ((input_length + 2) / 3);
    if ((int)buf_size<=output_length) return -1;

    for (unsigned int i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = 0;

    return output_length;
}


int base64_decode(const char *data, size_t input_length, char *decoded_data, size_t buf_size) {
    if (initialized_decoding_table == 0) build_decoding_table();

    if (input_length % 4 != 0) return -1;
    int output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') output_length--;
    if (data[input_length - 2] == '=') output_length--;
    if (output_length>(int)buf_size) return -2;

    for (unsigned int i = 0, j = 0; i < input_length;) {
        uint32_t sixtet_a = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sixtet_b = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sixtet_c = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];
        uint32_t sixtet_d = data[i] == '=' ? 0 & i++ : decoding_table[(unsigned char)data[i++]];

        uint32_t triple = (sixtet_a << 3 * 6)
        + (sixtet_b << 2 * 6)
        + (sixtet_c << 1 * 6)
        + (sixtet_d << 0 * 6);

        if ((int)j < output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if ((int)j < output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if ((int)j < output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    if(output_length<(int)buf_size) {
        decoded_data[output_length] = 0;
    }

    return output_length;
}


void build_decoding_table() {
    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
    initialized_decoding_table = 1;
}

std::string dia_read_file(const char * file_name) {
    //fprintf(stderr, "resource requested:[%s]\n", file_name);
    if (access(file_name, F_OK) != -1) {
        // file exists
        //fprintf(stderr, "[%s] found\n", file_name);
    } else {
        fprintf(stderr, "[%s] not found\n", file_name);
        return "";
    }

    std::string result;
    FILE *fp;
    fp = fopen(file_name, "r");
    if (fp == 0) {
        printf("error: can't read file [%s]\n", file_name);
        return result;
    }
    int nread = 1;

    #define DIA_READ_SIZE_BLOCK 1024
    char out[DIA_READ_SIZE_BLOCK];
    while (nread>0) {
        nread = fread(out, 1, DIA_READ_SIZE_BLOCK - 1, fp);
        if(nread>=DIA_READ_SIZE_BLOCK - 1) {
            out[DIA_READ_SIZE_BLOCK - 1] = 0;
        } else {
            out[nread] = 0;
        }
        result += out;
    }
    fclose(fp);
    return result;
}

std::string dia_get_resource(const char * folder_name, const char *resource_name) {
    std::string complete_filename = folder_name;
    complete_filename += "/";
    complete_filename += resource_name;
    return dia_read_file(complete_filename.c_str());
}

json_t * dia_get_resource_json(const char * folder_name, const char * resource_name) {
    std::string content = dia_get_resource(folder_name, resource_name);
    json_t *root;
    json_error_t error;
    root = json_loads(content.c_str(), 0, &error);

    if(!root) {
        printf("%s\n", resource_name );
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 0;
    }
    return root;
}

void dia_reverse(char * s)
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

class ScalerCoefs {
    public: 
    double StartCoordinate;
    double EndCoordinate;
    double StartPixel; 
    double EndPixel;
    double StartPixelPart;
    double EndPixelPart;
    
    ScalerCoefs(double startCoord, double endCoord, double pixelsTotal) {
        StartCoordinate = startCoord;
        EndCoordinate = endCoord;
        StartPixel = floor(StartCoordinate);
        EndPixel = floor(EndCoordinate);
        
        if (EndPixel>=pixelsTotal) {
            EndPixel=pixelsTotal-1;
        }        
        StartPixelPart = 1 + StartPixel - StartCoordinate;
        EndPixelPart = EndCoordinate - EndPixel;
    }
    void Print() {
        printf("<C:%.3g, >C:%.3g, <Pix:%.3g, >Pix:%.3g,<Prt:%.3g, >Prt:%.3g\n",
         StartCoordinate, EndCoordinate, StartPixel,
        EndPixel, StartPixelPart, EndPixelPart);
    }
    double GetPart(int coord) {
        double res = 1;
        if (coord==(int)StartPixel) {
            res = StartPixelPart;
        } else if (coord == (int)EndPixel) {
            res = EndPixelPart;
        } 
        return res;
    }
    
};


SDL_Surface *dia_ScaleSurface(SDL_Surface *Surface, Uint16 Width, Uint16 Height) {
    if(!Surface || !Width || !Height)
        return 0;

    SDL_Surface *_ret = SDL_CreateRGBSurface(Surface->flags, Width, Height, Surface->format->BitsPerPixel,
        Surface->format->Rmask, Surface->format->Gmask, Surface->format->Bmask, Surface->format->Amask);

//|_|_|_|_|
//|_|_|_|_|
//|_|_|_|_|
//|_|_|_|_|

    double scaleX = static_cast<double>(Surface->w) / static_cast<double>(Width);
    double scaleY = static_cast<double>(Surface->h) / static_cast<double>(Height);
    printf( "scale X: %.3g, scaleY: %.3g\n",scaleX, scaleY);
    
    for(int x=0;x<Width;x++) {
        // we have choosen a column here
        double xNewLeft = x;
        double xNewRight = x+1;
        ScalerCoefs xCoefs(xNewLeft * scaleX, xNewRight * scaleX, Surface->w);
        //xCoefs.Print();
        for (int y=0;y<Height;y++) {
            double yNewLeft = y;
            double yNewRight = y+1;
            ScalerCoefs yCoefs(yNewLeft * scaleY, yNewRight * scaleY, Surface->h);
            //yCoefs.Print();
            //exit(1);
            // let's loop original image
            double partsSum = 0;
            double Apart = 0;
            double Bpart = 0;
            double Cpart = 0;
            double Dpart = 0;
            
            double ApartSum = 0;
            double BpartSum = 0;
            double CpartSum = 0;
            double DpartSum = 0;
            
            for (int Xold = (int)xCoefs.StartPixel; Xold<=(int)xCoefs.EndPixel; Xold ++) {
                double xPart = xCoefs.GetPart(Xold);
                
                for (int Yold =(int)yCoefs.StartPixel;Yold<=(int)yCoefs.EndPixel; Yold ++) {
                    double yPart = yCoefs.GetPart(Yold);
                    
                    double part = xPart * yPart;
                    if (Xold>=Surface->w || Yold>=Surface->h) {
                        printf("READ: [%d:%d] of [%d,%d]\n", Xold, Yold, Surface->w, Surface->h);
                    }
                    //printf("size: [%d:%d] of [%d,%d]\n", Width, Height, Surface->w, Surface->h);
                    Uint32 pixel = ReadPixel(Surface, Xold, Yold);
                                        
                    Apart = (pixel & 0xFF000000) >> 24;
                    Bpart = (pixel & 0x00FF0000) >> 16;
                    Cpart = (pixel & 0x0000FF00) >> 8;
                    Dpart = pixel & 0x000000ff;
                    
                    //printf(" pixel: %x of %e:%e:%e:%e _ %d\n", pixel, Apart, Bpart, Cpart, Dpart);
                    ApartSum += Apart * part;
                    BpartSum += Bpart * part;
                    CpartSum += Cpart * part;
                    DpartSum += Dpart * part;
                    partsSum += part;
                    //printf("part: %e\n", part);
                }
            }
            if (partsSum <0.01 && partsSum>0.01) {
                partsSum = 1;
            }
            //printf("Apart:%e, sum:%e",ApartSum, partsSum);
            Uint32 newPixel = (int)(ApartSum / partsSum); 
            if (newPixel>255) {
                newPixel = 255;
            }
            newPixel = newPixel <<8;
            
            int newBPart = (int)(BpartSum / partsSum);
            if (newBPart>255) {
                newBPart = 255;
            }
            newPixel +=newBPart;
            newPixel = newPixel << 8;
            
            int newCPart = (int)(CpartSum / partsSum);
            if (newCPart>255) {
                newCPart = 255;
            }
            newPixel +=newCPart;
            newPixel = newPixel << 8;
            
            int newDPart = (int)(DpartSum / partsSum);
            if (newDPart>255) {
                newDPart = 255;
            }
            newPixel +=newDPart;
            DrawPixel(_ret, x, y, newPixel);
        }
    }
   
    return _ret;
}

char * dia_int_to_str(int n, char * result) {
     int i, sign;

     if ((sign = n) < 0) n = -n;
     i = 0;
     do {
         result[i++] = n % 10 + '0';
     } while ((n /= 10) > 0);
     if (sign < 0)
         result[i++] = '-';
     result[i] = '\0';
     dia_reverse(result);
     return result;
}

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

Uint32 ReadPixel(SDL_Surface* surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;
        break;

    case 2:
        return *(Uint16 *)p;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;
        break;

    case 4:
        return *(Uint32 *)p;
        break;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}
