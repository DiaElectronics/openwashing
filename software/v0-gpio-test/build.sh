g++ -o firmware.exe -g  dia_firmware.cpp dia_network.cpp dia_security.cpp dia_relayconfig.cpp \
dia_gpio.cpp dia_device.cpp request_collection.cpp \
dia_nv9usb.cpp dia_ccnet.cpp dia_devicemanager.cpp dia_screen.cpp \
SimpleRedisClient.cpp dia_functions.cpp \
-l:libcrypto.a -levent -lwiringPi -lpthread \
-I/usr/include/SDL `sdl-config --cflags` `sdl-config --libs` -lSDL -lSDL_image
