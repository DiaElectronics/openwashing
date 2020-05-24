#!/bin/bash

g++ -o opengles_test.exe -DRASPI opengles_test.cpp opengles/texture.cpp \
opengles/LoadShaders.cpp -I/opt/vc/include -L/opt/vc/lib -lbcm_host -lvcos -lGLESv2 -lEGL \
-I../app
