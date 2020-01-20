CC=g++

SRC=dia_firmware.cpp dia_microcoinsp.cpp dia_gpio.cpp dia_device.cpp dia_nv9usb.cpp dia_devicemanager.cpp dia_screen.cpp
SRC+=dia_configuration/dia_configuration.cpp dia_configuration/dia_screen_config.cpp dia_configuration/dia_screen_item.cpp
SRC+=SimpleRedisClient.cpp dia_functions.cpp dia_security.cpp
SRC+=dia_configuration/dia_screen_item_digits.cpp ./dia_screen/dia_int_pair.cpp ./dia_screen/dia_number.cpp ./dia_screen/dia_boolean.cpp
SRC+=./dia_screen/dia_font.cpp dia_configuration/dia_screen_item_image.cpp ./dia_screen/dia_string.cpp ./dia_runtime/dia_runtime.cpp
SRC+=dia_configuration/storage/dia_storage.cpp dia_configuration/storage/dia_storage_interface.cpp dia_ccnet.cpp
FLGS=-I. -I/usr/include/SDL -I./dia_screen -I./3rd/LuaBridge -I./3rd/lua53/include -I./dia_runtime -I./3rd -g
FLGS+=-I./dia_configuration -I./dia_configuration/storage

LIBS= -DCURL_STATICLIB -l:libevent.a -lwiringPi -lpthread `sdl-config --cflags` `sdl-config --libs` -lSDL -lSDL_image -l:libcrypto.a
LIBS+=-l:libjansson.a `curl-config --static-libs` ./3rd/lua53/src/liblua.a -ldl -Wall

prod:
	$(CC) -o firmware.exe $(SRC) $(FLGS) $(LIBS) -DUSE_GPIO
test:
	$(CC) -o firmware.exe.debug $(SRC) $(FLGS) $(LIBS)
