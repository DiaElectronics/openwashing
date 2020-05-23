// 2020, Roman Fedyashov
#ifndef _LEA_INTERFACE_H
#define _LEA_INTERFACE_H
#include <string>

namespace dia_app {
    class Error;
    class Application;

    // Let's never use inheritance... I hate it so much because of dependencies its creating...
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
}
#endif