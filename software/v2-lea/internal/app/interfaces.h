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
    class FloatPair;

    // Let's never use inheritance... I hate it so much because of dependencies its creating...
    // Application describes what application can do. 
    class Application {
        private:
            void * originalObject;
            Error *(*run)(void *);
            Error *(*stop)(void *);
            Error *(*loadFirmware)(void *, std::string);
            Error *(*setXMode)(void *, int);
        public:
        Error * Run() {
            return run(originalObject);
        }
        Error * Stop() {
            return stop(originalObject);
        }
        Error * LoadFirmware(std::string folder) {
            return loadFirmware(originalObject, folder);
        }
        // SetXMode is used to set a graphical mode for loading screens
        Error * SetXMode(int isGraphical) {
            return setXMode(originalObject, isGraphical);
        }
        Application(void * originalObject, Error *(*run)(void *), Error *(*stop)(void *), 
            Error *(*loadFirmware)(void *, std::string), Error *(*setXMode)(void *, int)) {
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
        int (*initScreen)(Size physicalSize. Size logicalSize);
        int (*displayImage)(img Image, FloatPair offset, FloatPair size);
        int (*swapFrame)();
    public:
        Error * InitScreen(originalObject, Size physicalSize. Size logicalSize) {
            return initScreen(originalObject, physicalSize, logicalSize);
        }
        Error * DisplayImage(originalObject, img Image, FloatPair offset, FloatPair size) {
            return displayImage(img, offset, size);
        }
        void SwapFrame() {
            return swapFrame(originalObject);
        }
        Renderer(void *_originalObject,int (*_initScreen)(Size physicalSize. Size logicalSize), 
        int (*_displayImage)(img Image, FloatPair offset, FloatPair size), int (*_swapFrame)()) {
            originalObject = _originalObject;
            initScreen = _initScreen;
            displayImage = _displayImage;
            swapFrame = _swapFrame;
            if (!originalObject || !initScreen || !displayImage || !swapFrame ) {
                printf("ERROR: Renderer got NULL parameter and will fail soon!\n");
            }
        }
    };
    
    // FloatPair is just a pair of floats... using for coordinates or 
    // rectangle size
    class FloatPair {
    private:
        float x, y;
    public:
        inline float X() { return X;}
        inline float Y() { return Y;}
        inline float SetX(float X) { return x=X; }
        inline float SetY(float Y) { return y=Y; }
        FloatPair() { x = 0; y = 0; }
        FloatPair(float X, Y) { x=X;y=Y; }
    };
    
    // Image is something that can be displayed by Renderer
    class Image {
    private:
        FloatPair size;
    public:
        inline FloatPair Size() { return size; } 
        Image() {
        };
    }
}
#endif
