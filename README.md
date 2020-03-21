# OpenWashing

**OpenWashing** is a project to create an electronics and software for self-service car washing systems. The software is distributed for free under the GPL-2.0 license. Project uses an open source libraries: SDL (https://www.libsdl.org/), LuaBridge (https://github.com/vinniefalco/LuaBridge), WiringPi (https://github.com/WiringPi/WiringPi), LibCurl (https://curl.haxx.se/libcurl/).

This project is a part of carwashing ecosystem, developing by OpenRobots. 

## Features

- Reliable runtime software, written in C and C++.
- Integration of modern payment systems: banknote, coin and NFC card readers. 
- User-friendly interface for car wash clients.
- Support of fiscalization, according to the laws of the Russian Federation.
- Easy-editable car wash system behaviour, based on Lua scripts.
- Fast deployment on single board computers, such as Raspberry Pi 3B.
    
## Requirements
- Any GNU/Linux distribution for testing; Raspbian for deployment.

## Download and Install
- Clone this repository to any directory.
- Execute ./prepare_pi.sh script in /software/v1-enlight
