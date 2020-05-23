CC=g++

SRC=main.cpp

prod:
	$(CC) -o firmware.exe $(SRC)
test:
	$(CC) -o firmware_dbg.exe -O0 -ggdb3 $(SRC) -DSKIP_GPIO
