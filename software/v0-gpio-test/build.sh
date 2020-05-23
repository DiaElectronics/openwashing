g++ -o firmware.exe -g  dia_firmware.cpp dia_network.cpp dia_relayconfig.cpp dia_gpio.cpp dia_device.cpp request_collection.cpp \
dia_nv9usb.cpp dia_devicemanager.cpp dia_screen.cpp -levent -lwiringPi -lpthread \
-I/usr/include/SDL `sdl-config --cflags` `sdl-config --libs` -lSDL -lSDL_image
