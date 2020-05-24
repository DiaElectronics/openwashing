#!/bin/bash

g++ -o opengles_test.exe opengles_test.cpp opengles/texture.cpp opengles/startScreen.cpp \
opengles/LoadShaders.cpp -I/opt/vc/include -L/opt/vc/lib -lbcm_host -lvcos -lGLESv2 -lEGL 
