// 2020, Roman Fedyashov
// This header can be used in DAL, DEVICES etc. layers. The application must never know about DAL etc.
#ifndef _LEA_INTERFACE_H
#define _LEA_INTERFACE_H
#include <string>
#include <stdio.h>

namespace DiaApp {
    class Error;
    class Application;
    class Renderer;
    class Image;
    
    // FloatPair is just a pair of floats... using for coordinates or 
    // rectangle size
    class FloatPair {
    private:
        float x, y;
    public:
        inline float X() { return x;}
        inline float Y() { return y;}
        inline float SetX(float X) { return x=X; }
        inline float SetY(float Y) { return y=Y; }
        FloatPair() { x = 0; y = 0; }
        FloatPair(float _X, float _Y) { x=_X;y=_Y; }
    };

    // Let's never use inheritance... I hate it so much because of dependencies its creating...
    // Application describes what application can do. 
    class Application {
        private:
            void * originalObject;
            int(*run)(void *);
            int(*stop)(void *);
            int(*loadFirmware)(void *, std::string);
            int(*setXMode)(void *, int);
        public:
        int Run() {
            return run(originalObject);
        }
        int Stop() {
            return stop(originalObject);
        }
        int LoadFirmware(std::string folder) {
            return loadFirmware(originalObject, folder);
        }
        // SetXMode is used to set a graphical mode for loading screens
        int SetXMode(int isGraphical) {
            return setXMode(originalObject, isGraphical);
        }
        Application(void * originalObject, int(*run)(void *), int(*stop)(void *), 
            int(*loadFirmware)(void *, std::string), int(*setXMode)(void *, int)) {
        }
    };
    
    // Error will be returned as an Error from different methods
    class Error {
    private:
        std::string message;
        int code;
    public:
        Error(std::string Message) {
            message = message;
            code = 0;
        }
        Error(std::string Message, int Code) {
            message = message;
            code = 0;
        }
        std::string Message() {
            return message;
        }
        int Code() {
            return code;
        }
    };
    
    // Renderer will be responsible for Rendering objects on the screen
    class Renderer {
    private:
        void *originalObject;
        int (*initScreen)(void *, FloatPair);
        int (*displayImage)(void *, Image* , FloatPair, FloatPair);
        void (*swapFrame)(void *);
    public:
        int InitScreen(void *originalObject, FloatPair logicalSize) {
            return initScreen(originalObject, logicalSize);
        }
        int DisplayImage(void *originalObject, Image *img, FloatPair offset, FloatPair size) {
            return displayImage(originalObject, img, offset, size);
        }
        void SwapFrame() {
            return swapFrame(originalObject);
        }
        Renderer(void *_originalObject,int (*_initScreen)(void *, FloatPair), 
        int (*_displayImage)(void *, Image*, FloatPair, FloatPair), void (*_swapFrame)(void *)) {
            originalObject = _originalObject;
            initScreen = _initScreen;
            displayImage = _displayImage;
            swapFrame = _swapFrame;
            if (!originalObject || !initScreen || !displayImage || !swapFrame ) {
                printf("ERROR: Renderer got NULL parameter and will fail soon!\n");
            }
        }
    };
    
    
    
    // Image is something that can be displayed by Renderer
    class Image {
    private:
        FloatPair size;
    public:
        inline FloatPair Size() { return size; } 
        Image() {
        }
    };
}
#endif
